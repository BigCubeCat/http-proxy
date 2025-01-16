#pragma once
#include <vector>

#include <sys/epoll.h>

#include "thread_pool.hpp"

/*!
 * \brief Объект прокси
 * Принимает конфиг и работает с пулом потоков
 */
class http_proxy_t {
private:
    int m_port        = 8080;
    bool m_is_running = false;

    int m_count_workers;
    int m_listen_fd;
    std::vector<std::shared_ptr<worker_iface>> m_workers;

    std::shared_ptr<thread_pool_t> m_pool;

    [[nodiscard]] bool init_listen_socket(int listen_fd) const;

    /*!
     * Метод для главного воркера
     */
    int accept_client();

public:
    explicit http_proxy_t(int port, int count_threads);

    void run();

    void stop(int status);
};
