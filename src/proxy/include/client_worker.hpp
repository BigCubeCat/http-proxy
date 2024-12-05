#pragma once

#include <queue>

#include "task.hpp"

/*!
 * \brief Обработчик подключений, запускаемый в отдельной нити
 */
class client_worker : public worker_iface {
private:
    std::queue<int> m_fd;

public:
    explicit client_worker(std::queue<int> m_fd) : m_fd(std::move(m_fd)) { }
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
