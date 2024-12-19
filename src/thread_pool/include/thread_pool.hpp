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
 */
class thread_pool_t {
private:
    size_t m_count_pools;
    size_t m_next_thread = 0;

    std::vector<std::thread> m_threads;
    std::vector<std::shared_ptr<worker_iface>> m_tasks;
    std::unordered_map<int, size_t> m_fd_map;    // связь [fd] -> [thread index
                                                 // in pool]

public:
    explicit thread_pool_t(std::vector<std::shared_ptr<worker_iface>> tasks);

    /*!
     * \brief Запускает потоки
     */
    void run(const std::function<void *(void *)> &start_routine);

    void stop();

    /*!
     * \brief Добавляет файловый дискриптор в обработку
     */
    void add_task(int fd);

    /*!
     * \brief Сообщет пулу потоков об активности дескриптора
     */
    void notify(int fd);

    ~thread_pool_t();
};
