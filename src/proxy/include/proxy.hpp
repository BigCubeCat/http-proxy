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
    int m_port        = 8080;
    bool m_is_running = false;

    int m_count_workers;
    std::vector<std::shared_ptr<worker_iface>> m_workers;

    cache_t *m_cache;
    std::shared_ptr<thread_pool_t> m_pool;

    bool init_listen_socket(int listen_fd);

public:
    explicit http_proxy_t(cache_t *cache, int port, int count_threads);

    void run();

    void stop(int status);
};
