#pragma once
#include <vector>

#include <sys/epoll.h>

#include "proxy_cache.hpp"
#include "thread_pool.hpp"

/*!
 * Объект прокси
 */
class http_proxy_t {
private:
    int m_exit_fd       = -1;
    int m_epoll_fd      = -1;
    int m_listen_fd     = -1;
    int m_port          = 8080;
    int m_server_socket = -1;
    bool m_is_running   = false;

    std::vector<epoll_event> m_events;

    int m_count_workers;
    std::vector<std::shared_ptr<worker_iface>> m_workers;

    cache_t *m_cache;
    std::shared_ptr<thread_pool_t> m_pool;

    [[nodiscard]] int accept_client() const;

public:
    explicit http_proxy_t(cache_t *cache, int port, int count_threads);

    void run();

    void stop(int status);
};
