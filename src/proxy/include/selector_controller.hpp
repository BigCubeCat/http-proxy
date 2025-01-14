#pragma once


#include <vector>

#include <sys/epoll.h>


/*!
 * \brief объект, хранящий в себе данные для epoll
 * хранит дискприптор epoll и позволяет регистрировать новые fd
 */
class selector_controller {
private:
    int m_epoll_fd = -1;

    std::vector<epoll_event> m_events;

public:
    selector_controller();
    selector_controller(const selector_controller &)            = default;
    selector_controller(selector_controller &&)                 = delete;
    selector_controller &operator=(const selector_controller &) = default;
    selector_controller &operator=(selector_controller &&)      = delete;
    explicit selector_controller(std::vector<epoll_event> m_events)
        : m_events(std::move(m_events)) { }
    ~selector_controller();

    void register_fd(int fd, uint32_t op) noexcept;

    void change_fd_mode(int fd, uint32_t op) noexcept;

    void unregister_fd(int fd) noexcept;

    int select() noexcept;

    epoll_event &operator[](size_t i) {
        return m_events.at(i);
    };

    [[nodiscard]] int epoll_fd() const {
        return m_epoll_fd;
    }
};
