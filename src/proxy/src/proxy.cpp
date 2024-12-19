#include "proxy.hpp"

#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>

#include "client_worker.hpp"
#include "const.hpp"
#include "network.hpp"
#include "status_check.hpp"
#include "thread_pool.hpp"


http_proxy_t::http_proxy_t(cache_t *cache, int port, int count_threads)
    : m_port(port),
      m_events(MAX_EVENTS),
      m_count_workers(count_threads),
      m_workers(count_threads),
      m_cache(cache) { }

void http_proxy_t::run() {
    m_is_running = true;
    m_listen_fd  = socket(AF_INET, SOCK_STREAM, 0);
    if (!error_status(m_listen_fd, "socket create failed")) {
        return;
    }
    if (!error_status(bind_socket(m_listen_fd, m_port), "bind failed")) {
        return;
    }
    if (!error_status(listen(m_listen_fd, SOMAXCONN), "listen failed")) {
        return;
    }
    set_not_blocking(m_listen_fd);
    m_epoll_fd = epoll_create1(0);
    if (!error_status(m_epoll_fd, "epoll_create1 failed")) {
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
    int nfds;
    while (m_is_running) {
        nfds = epoll_wait(
            m_epoll_fd, m_events.data(), MAX_EVENTS, EPOLL_WAIT_TIMEOUT
        );
        if (nfds < 0 || !m_is_running) {
            break;
        }
        for (int i = 0; i < nfds; ++i) {
            spdlog::trace("обработка {} из {}", i, nfds);
            if (m_events[i].data.fd == m_listen_fd) {
                spdlog::trace("серверный дескриптор");
                auto client_fd = accept_client();
                if (!error_status(client_fd, "failed to accept client")) {
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
    warn_status(close(m_listen_fd), "error on close listen fd");
    m_pool->stop();
}

int http_proxy_t::accept_client() const {
    auto client_fd = accept(m_listen_fd, nullptr, nullptr);
    if (!error_status(client_fd, "accept failed")) {
        return -1;
    }
    set_not_blocking(client_fd);
    register_fd(m_epoll_fd, client_fd);
    return client_fd;
}

void http_proxy_t::stop(int status) {
    spdlog::debug("exiting with status {}", status);
    m_is_running = false;
    m_pool->stop();
    spdlog::trace("stop");
}
