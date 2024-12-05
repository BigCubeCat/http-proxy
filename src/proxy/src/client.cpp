#include "client.hpp"

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "picohttpparser.h"

// Максимальное количество клиентов
constexpr int MAX_EVENTS  = 10;
constexpr int BUFFER_SIZE = 4096;
constexpr int CACHE_TTL   = 300;    // Время жизни кеша в секундах

// Структура для хранения кешированных данных
struct CacheEntry {
    std::string data;
    time_t timestamp;
};

// Глобальный кеш
std::unordered_map<std::string, CacheEntry> cache;

// Утилита: установка сокета в неблокирующий режим
void setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}

// Парсинг запроса с помощью picohttpparser
bool parseRequest(
    const char *buf,
    int len,
    std::string &method,
    std::string &url,
    std::string &host
) {
    const char *method_ptr, *path_ptr;
    size_t method_len, path_len;
    struct phr_header headers[100];
    size_t num_headers = sizeof(headers) / sizeof(headers[0]);
    int minor_version;

    int pret = phr_parse_request(
        buf,
        len,
        &method_ptr,
        &method_len,
        &path_ptr,
        &path_len,
        &minor_version,
        headers,
        &num_headers,
        0
    );

    if (pret <= 0)
        return false;

    method = std::string(method_ptr, method_len);
    url    = std::string(path_ptr, path_len);

    for (size_t i = 0; i < num_headers; ++i) {
        if (strncasecmp(headers[i].name, "Host", headers[i].name_len) == 0) {
            host = std::string(headers[i].value, headers[i].value_len);
        }
    }

    return true;
}

// Резолвинг хоста (DNS)
bool resolveHost(const std::string &host, std::string &ip_address) {
    struct addrinfo hints {}, *res;
    hints.ai_family   = AF_INET;    // IPv4
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0) {
        perror("getaddrinfo");
        return false;
    }

    char ip[INET_ADDRSTRLEN];
    if (inet_ntop(
            AF_INET,
            &((struct sockaddr_in *)res->ai_addr)->sin_addr,
            ip,
            sizeof(ip)
        )
        == nullptr) {
        perror("inet_ntop");
        freeaddrinfo(res);
        return false;
    }

    ip_address = ip;
    freeaddrinfo(res);
    return true;
}

// Отправка запроса на целевой сервер
std::string
forwardRequest(const std::string &host, const std::string &request) {
    std::string ip_address;
    if (!resolveHost(host, ip_address)) {
        std::cerr << "Failed to resolve host: " << host << std::endl;
        return "";
    }

    struct sockaddr_in server_addr {};
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return "";
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(80);    // HTTP порт
    if (inet_pton(AF_INET, ip_address.c_str(), &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return "";
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr))
        < 0) {
        perror("Connection to server failed");
        close(sock);
        return "";
    }

    send(sock, request.c_str(), request.size(), 0);

    char buffer[BUFFER_SIZE];
    std::ostringstream response;
    int bytes_read;
    while ((bytes_read = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        response.write(buffer, bytes_read);
    }

    close(sock);
    return response.str();
}

// Основная логика работы прокси-сервера
void runProxyServer(int port) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr {};
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(port);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))
        < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, SOMAXCONN) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    setNonBlocking(listen_fd);

    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("Epoll creation failed");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev {}, events[MAX_EVENTS];
    ev.events  = EPOLLIN;
    ev.data.fd = listen_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) < 0) {
        perror("Epoll_ctl failed");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    while (true) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            perror("Epoll wait failed");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == listen_fd) {
                int client_fd = accept(listen_fd, nullptr, nullptr);
                if (client_fd < 0) {
                    perror("Accept failed");
                    continue;
                }
                setNonBlocking(client_fd);

                ev.events  = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
            }
            else {
                int client_fd  = events[i].data.fd;
                int bytes_read = read(client_fd, buffer, BUFFER_SIZE);
                if (bytes_read <= 0) {
                    close(client_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
                }
                else {
                    std::string method, url, host;
                    if (!parseRequest(buffer, bytes_read, method, url, host)) {
                        close(client_fd);
                        continue;
                    }

                    if (method == "GET") {
                        auto it = cache.find(url);
                        if (it != cache.end()
                            && (time(nullptr) - it->second.timestamp)
                                   < CACHE_TTL) {
                            send(
                                client_fd,
                                it->second.data.c_str(),
                                it->second.data.size(),
                                0
                            );
                        }
                        else {
                            std::string response = forwardRequest(host, buffer);
                            if (!response.empty()) {
                                cache[url] = { response, time(nullptr) };
                                send(
                                    client_fd,
                                    response.c_str(),
                                    response.size(),
                                    0
                                );
                            }
                        }
                    }

                    close(client_fd);
                }
            }
        }
    }

    close(listen_fd);
}
//
// int main() {
//     const int port = 8080;
//     runProxyServer(port);
//     return 0;
// }
