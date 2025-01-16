#pragma once

#include <spdlog/spdlog.h>
#include <sys/epoll.h>

#include "proxy_server_t.hpp"
#include "task.hpp"
#include "thread_pool.hpp"

/*!
 * \brief Обработчик подключений, запускаемый в отдельной нити
 */
class client_worker : public worker_iface {
private:
    bool m_is_root;
    bool m_worker_is_running = true;
    int m_listen_fd          = -127;

    proxy_server_t m_proxy_inst;

    thread_pool_t *m_pool;

    void process_client_fd(int client_fd, uint32_t events);

public:
    explicit client_worker(thread_pool_t *pool_ptr, int listen_fd)
        : m_is_root(true), m_listen_fd(listen_fd), m_pool(pool_ptr) { }

    explicit client_worker(thread_pool_t *pool_ptr)
        : m_is_root(false), m_pool(pool_ptr) { }

    void start() override;

    void stop() override;

    /*!
     * Добавить файловый регистр на выполнение операции
     */
    void add_task(int fd) override;

    /*!
     * пробудить файловый дискриптор
     */
    void toggle_task(int fd) override { };

    virtual ~client_worker() = default;
};

/*!
 * Поточная функция для запуска объекта client-worker
 */
void *start_client_worker_routine(void *arg);
