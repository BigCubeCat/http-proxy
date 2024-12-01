#pragma once

#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include "task.hpp"

/*!
 * thread_pool_t
 * Реализация пула потоков.
 * Распределяет задачи между потоками по алгоритму round-robin
 * При инициализации принимает
 * вектор указателей на интерфейс таски
 * и число потоков
 * (по умолчанию hardware_concurrency())
 * Не имеет функционала завершения задачи в силу исходной задачи
 */
class thread_pool_t {
private:
    size_t m_count_pools;
    size_t m_next_thread = 0;

    std::vector<std::thread> m_threads;
    std::vector<std::shared_ptr<worker_iface>> m_tasks;

public:
    explicit thread_pool_t(
        const std::vector<std::shared_ptr<worker_iface>> &tasks
    );

    /*!
     * \brief Запускает потоки
     */
    void run(const std::function<void *(void *)> &start_routine);

    void add_task(void *arg);
};
