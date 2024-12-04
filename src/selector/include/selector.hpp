#pragma once

#include <vector>

#include <sys/epoll.h>

/*!
 * \brief объект для мультиплексирования файловых дискрипторов
 * является оберткой на epoll
 */
class selector_t {
private:
    int m_epoll_fd;
    /*!
     * вектор с файловыми дискрипторами.
     * размер - максимальное число дискрипторов
     */
    std::vector<epoll_event> m_events;
    int m_count_events;

    void selector_epoll_ctl(int fd, int operation, uint32_t);

public:
    explicit selector_t(int count_events);

    /*!
     * добавление файлового дискриптора
     */
    void append_fd(int fd, uint32_t op);

    /*!
     * удаление файловго дискриптора
     */
    void delete_fd(int fd);

    /*!
     * обновление файловго дискриптора
     */
    void update_fd(int fd, uint32_t op);

    int select();

    epoll_event &operator[](size_t i) {
        return m_events[i];
    }

    ~selector_t();
};
