#include "proxy.hpp"

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
#include "thread_pool.hpp"
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
    sockaddr_in server_addr {};
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(m_port);
    auto bind_status            = bind(
        m_listen_fd,
        reinterpret_cast<sockaddr *>(&server_addr),
        sizeof(server_addr)
    );
    if (bind_status < 0) {
        spdlog::critical("bind failed");
        return;
    }
    if (listen(m_listen_fd, SOMAXCONN) < 0) {
        spdlog::critical("listen failed");
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

    spdlog::info("count workers {}", m_count_workers);
    m_workers.resize(m_count_workers);
    for (int i = 0; i < m_count_workers; ++i) {
        m_workers[i] = std::shared_ptr<worker_iface>(
            std::make_shared<client_worker>(m_cache, m_events, m_epoll_fd)
        );
    }
    m_pool = std::make_shared<thread_pool_t>(m_workers);
    m_pool->run(start_client_worker_routine);

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
                auto client_fd = accept_client();
                if (client_fd <= 0) {
                    spdlog::error("failed to accept client");
                    continue;
                }
                m_pool->add_task(client_fd);
            }
            else {
                spdlog::trace("клиентский дескриптор");
                m_pool->notify(m_events[i].data.fd);
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

int http_proxy_t::accept_client() const {
    auto client_fd = accept(m_listen_fd, nullptr, nullptr);
    if (client_fd < 0) {
        spdlog::warn("accept failed");
        return -1;
    }
    set_not_blocking(client_fd);
    epoll_event ev {};
    ev.events  = EPOLLIN | EPOLLET;
    ev.data.fd = client_fd;
    hs(epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev), "epoll_ctl ADD");
    return client_fd;
}
