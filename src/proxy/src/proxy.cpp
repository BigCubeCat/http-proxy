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
#include "network.hpp"
#include "status_check.hpp"
#include "thread_pool.hpp"


http_proxy_t::http_proxy_t(cache_t *cache, int port, int count_threads)
    : m_port(port),
      m_count_workers(count_threads - 1),
      m_workers(count_threads),
      m_cache(cache) { }

void http_proxy_t::run() {
    m_pool        = std::make_shared<thread_pool_t>(m_count_workers + 1);
    m_is_running  = true;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (!init_listen_socket(listen_fd)) {
        return;
    }
    auto root_worker =
        std::make_shared<client_worker>(m_cache, m_pool.get(), listen_fd);
    root_worker->add_task(listen_fd);

    spdlog::info("count workers {}", m_count_workers);
    m_workers.resize(m_count_workers + 1);
    m_workers[0] = root_worker;
    for (int i = 1; i < m_count_workers + 1; ++i) {
        m_workers[i] = std::shared_ptr<worker_iface>(
            std::make_shared<client_worker>(m_cache, m_pool.get())
        );
    }
    m_pool->set_tasks(m_workers);
    root_worker->start();
    m_pool->run(start_client_worker_routine);

    warn_status(close(listen_fd), "error on close listen fd");
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
    try {
        set_not_blocking(listen_fd);
    }
    catch (const std::exception &e) {
        spdlog::critical("{}", e.what());
        return false;
    }
    return true;
}
