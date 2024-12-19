#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>

#include <spdlog/spdlog.h>
#include <sys/epoll.h>

#include "cache.hpp"
#include "proxy_cache.hpp"
#include "task.hpp"

/*!
 * \brief Обработчик подключений, запускаемый в отдельной нити
 */
class client_worker : public worker_iface {
private:
    bool m_worker_is_running = true;
    /// Файловый дискриптор, из proxy
    int m_epoll_fd = -1;
    std::vector<epoll_event> m_events;
    std::queue<int> m_fds;
    std::mutex m_lock;

    std::condition_variable m_toggled_cond;
    std::mutex m_toggle_lock;

    cache_t *m_cache;

public:
    explicit client_worker(
        cache_t *cache, const std::vector<epoll_event> &events, int epoll_fd
    )
        : m_epoll_fd(epoll_fd), m_events(events), m_cache(cache) { }

    void process_client_fd(int client_fd) const;

    void start() override;

    void stop() override;

    /*!
     * Добавить файловый регистр на выполнение операции
     */
    void add_task(int fd) override;

    /*!
     * пробудить файловый дискриптор
     */
    void toggle_task(int fd) override;

    virtual ~client_worker() = default;
};

/*!
 * Поточная функция для запуска объекта client-worker
 */
void *start_client_worker_routine(void *arg);
