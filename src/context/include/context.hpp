#pragma once

#include <vector>

#include <sys/epoll.h>


/*!
 * \brief класс для работы с epoll
 * регистрирует и удаляет файловые дискрипторы
 */
class context_t {
    int m_ep_fd;

    std::vector<epoll_event> m_events;

    void update() noexcept;

public:
    explicit context_t();

    void register_file_descriptor(int fd, uint32_t op) const noexcept;

    void change_descriptor_mode(int fd, uint32_t op) const noexcept;

    void unregister_file_descriptor(int fd) const noexcept;

    int do_select() noexcept;

    epoll_event &operator[](size_t i) {
        return m_events[i];
    };


    ~context_t();
};
