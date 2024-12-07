#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>

#include <sys/epoll.h>

#include "task.hpp"

/*!
 * \brief Обработчик подключений, запускаемый в отдельной нити
 */
class client_worker : public worker_iface {
private:
    /// Файловый дискриптор, из proxy
    int m_epoll_fd = -1;
    std::vector<epoll_event> m_events;
    std::queue<int> m_fds;
    std::mutex m_lock;

    std::condition_variable m_toggled_cond;
    std::mutex m_toggle_lock;

public:
    explicit client_worker(const std::vector<epoll_event> &events, int epoll_fd)
        : m_epoll_fd(epoll_fd), m_events(events) { }

    void process_client_fd(int client_fd) const;

    void start() override;

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
