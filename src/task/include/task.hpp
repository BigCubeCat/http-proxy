#pragma once

/*!
 * task_iface
 * Интерфейс задачи, запускаемой в thread_pool
 * например для прокси start_routine будет конструировать прокси, а add_task
 * додавлять соединение
 */
class worker_iface {
public:
    /*!
     * \brief Функция, запускаемая в отдельном потоке
     */
    virtual void start() = 0;
    /*!
     * \brief добавление подзадачи
     */
    virtual void add_task(void *arg) = 0;
};