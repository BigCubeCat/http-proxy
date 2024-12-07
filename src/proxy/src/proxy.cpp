#include "proxy.hpp"

#include <array>
#include <cstring>
#include <memory>
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

#include "const.hpp"
#include "parser.hpp"
#include "utils.hpp"

#include "proxy/proxy_runtime_exception.hpp"

http_proxy_t::http_proxy_t(
    lru_cache_t<std::string> *cache, int port, int count_threads
)
    : m_port(port),
      m_events(MAX_EVENTS),
      m_count_workers(count_threads),
      m_workers(count_threads),
      m_cache(cache) { }

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

    m_workers.reserve(m_count_workers);
    m_workers.push_back(std::make_shared<client_worker>(m_events, m_epoll_fd));
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
                m_workers[0]->process_client_fd(m_events[i].data.fd);
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
