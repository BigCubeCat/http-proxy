#include "proxy.hpp"

#include <array>
#include <cstring>
#include <string>
#include <vector>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "parser.hpp"
#include "utils.hpp"

#include "proxy/proxy_runtime_exception.hpp"


http_proxy_t::http_proxy_t(int port) : m_port(port), m_events(MAX_EVENTS) { }

void http_proxy_t::run() {
    m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listen_fd < 0) {
        spdlog::critical("socket creation failed");
        return;
    }
    struct sockaddr_in server_addr {};
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(m_port);
    if (bind(m_listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))
        < 0) {
        spdlog::critical("Bind failed");
        return;
    }
    if (listen(m_listen_fd, SOMAXCONN) < 0) {
        spdlog::critical("Listen failed");
        return;
    }
    set_not_blocking(m_listen_fd);

    m_epoll_fd = epoll_create1(0);
    if (m_epoll_fd < 0) {
        spdlog::critical("epoll creation failed");
        return;
    }

    epoll_event ev {};
    ev.events  = EPOLLIN;
    ev.data.fd = m_listen_fd;

    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_listen_fd, &ev) < 0) {
        spdlog::critical("epoll_ctl failed");
        return;
    }
    while (true) {
        int nfds = epoll_wait(m_epoll_fd, m_events.data(), MAX_EVENTS, -1);
        if (nfds < 0) {
            spdlog::error("epoll wait failed");
            return;
        }
        spdlog::debug("request {}", nfds);

        for (int i = 0; i < nfds; ++i) {
            spdlog::trace("обработка {} из {}", i, nfds);
            if (m_events[i].data.fd == m_listen_fd) {
                spdlog::trace("серверный дескриптор");
                accept_client();
            }
            else {
                spdlog::trace("клиентский дескриптор");
                process_client_fd(i);
            }
        }
    }
    close(m_listen_fd);
}

void http_proxy_t::set_not_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        spdlog::critical("fcntl F_GETFL");
        throw proxy_runtime_exception("fcntl F_GETFL", -1);
    }
    flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (flags == -1) {
        spdlog::critical("fcntl F_SETFL");
        throw proxy_runtime_exception("fcntl F_SETFL", -1);
    }
}

void http_proxy_t::accept_client() const {
    auto client_fd = accept(m_listen_fd, nullptr, nullptr);
    if (client_fd < 0) {
        spdlog::warn("accept failed");
        return;
    }
    set_not_blocking(client_fd);
    epoll_event ev {};
    ev.events  = EPOLLIN | EPOLLET;
    ev.data.fd = client_fd;
    hs(epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev), "epoll_ctl ADD");
}

void http_proxy_t::process_client_fd(int i) {
    // TODO: засунуть это в воркера
    auto client_fd = m_events[i].data.fd;
    std::array<char, BUFFER_SIZE> buffer {};
    auto bytes_read = read(client_fd, buffer.data(), BUFFER_SIZE);
    if (bytes_read <= 0) {
        hs(close(client_fd), "end connection");
        hs(epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr),
           "epoll_ctl DEL");
    }
    else {
        std::string method;
        std::string url;
        std::string host;
        if (!parse_request(buffer.data(), bytes_read, method, url, host)) {
            hs(close(client_fd), "close");
            return;
        }

        if (method == "GET") {
            spdlog::trace("GET method");
            // тут надо сделать запрос к кэшу
            // auto it = cache.find(url);
            auto response = forward_request(host, buffer.data());
            if (!response.empty()) {
                // cache[url] = { .data = response, .timestamp = time(nullptr)
                // };
                hs(static_cast<int>(
                       send(client_fd, response.c_str(), response.size(), 0)
                   ),
                   "send in process_client_fd");
            }
        }
        hs(close(client_fd), "close");
    }
}
