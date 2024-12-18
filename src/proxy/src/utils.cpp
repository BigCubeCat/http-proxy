#include "utils.hpp"

#include <cstring>
#include <sstream>
#include <string>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "const.hpp"
#include "parser.hpp"

/*!
 * \brief Отправка запроса на целевой сервер
 */
std::string
forward_request(const std::string &host, const std::string &request) {
    std::string ip_address;
    if (!resolve_host(host, ip_address)) {
        spdlog::error("failed to resolve host: {}", host);
        return "";
    }

    struct sockaddr_in server_addr {};
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        spdlog::error("Socket creation failed");
        return "";
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(80);    // HTTP порт
    if (inet_pton(AF_INET, ip_address.c_str(), &server_addr.sin_addr) <= 0) {
        spdlog::error("inet_pton");
        close(sock);
        return "";
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr))
        < 0) {
        spdlog::error("Connection to server failed");
        close(sock);
        return "";
    }

    hs(static_cast<int>(send(sock, request.c_str(), request.size(), 0)),
       "send in forward");

    std::array<char, BUFFER_SIZE> buffer {};
    std::ostringstream response;
    ssize_t bytes_read = recv(sock, buffer.data(), BUFFER_SIZE, 0);
    spdlog::trace("bytes_read = {}", bytes_read);
    while (bytes_read > 0) {
        response.write(buffer.data(), bytes_read);
        bytes_read = recv(sock, buffer.data(), BUFFER_SIZE, 0);
        // spdlog::trace("bytes_read = {}", bytes_read);
    }

    std::string a;
    for (int i = 0; i < 500; ++i) {
        a += response.str()[i];
    }
    spdlog::warn("header = {}", a);

    hs(close(sock), "close error");
    spdlog::warn("response size = {}", response.str().size());
    return response.str();
}

void hs(int status, const std::string &msg) {
    if (status != 0) {
        spdlog::warn("{}: {}", msg, status);
    }
}
