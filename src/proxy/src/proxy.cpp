#include "proxy.hpp"

#include <cstring>
#include <memory>
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
#include "network.hpp"
#include "status_check.hpp"
#include "thread_pool.hpp"


http_proxy_t::http_proxy_t(int port, int count_threads)
    : m_port(port), m_count_workers(count_threads), m_workers(count_threads) { }

void http_proxy_t::run() {
    m_pool       = std::make_shared<thread_pool_t>(m_count_workers);
    m_is_running = true;
    m_listen_fd  = socket(AF_INET, SOCK_STREAM, 0);
    // создаем серверный сокет
    if (!init_listen_socket(m_listen_fd)) {
        return;
    }
    spdlog::trace("listen socket inited");

    spdlog::info("count workers {}", m_count_workers);
    m_workers.resize(m_count_workers);
    for (int i = 0; i < m_count_workers; ++i) {
        spdlog::trace("init worker {}", i);
        m_workers[i] = std::shared_ptr<worker_iface>(
            std::make_shared<client_worker>(m_pool.get())
        );
    }

    spdlog::debug("all workers initialized");

    m_pool->set_tasks(m_workers);
    m_pool->run(start_client_worker_routine);

    while (m_is_running) {
        auto status = accept_client();
        if (status < 0) {
            continue;
        }
        m_pool->add_task(status);
    }

    warn_status(close(m_listen_fd), "error on close listen fd");
}

void http_proxy_t::stop(int status) {
    spdlog::debug("exiting with status {}", status);
    m_is_running = false;
    m_pool->stop();
    spdlog::trace("stop");
}

bool http_proxy_t::init_listen_socket(int listen_fd) const {
    if (!error_status(listen_fd, "socket create failed")) {
        return false;
    }
    if (!error_status(bind_socket(listen_fd, m_port), "bind failed")) {
        return false;
    }
    if (!error_status(listen(listen_fd, SOMAXCONN), "listen failed")) {
        return false;
    }
    return set_not_blocking(listen_fd);
}

int http_proxy_t::accept_client() {
    auto client_fd = accept(m_listen_fd, nullptr, nullptr);
    if (client_fd < 0) {
        return client_fd;
    }
    if (!set_not_blocking(client_fd)) {
        return -2;
    }
    m_pool->add_task(client_fd);
    return client_fd;
}
