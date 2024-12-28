#pragma once


#include <vector>

#include <sys/epoll.h>


/*!
 * \brief объект, хранящий в себе данные для epoll
 * хранит дискприптор epoll и позволяет регистрировать новые fd
 */
class epoll_controller {
private:
    int m_epoll_fd = -1;

    std::vector<epoll_event> m_events;

public:
    epoll_controller();
    epoll_controller(const epoll_controller &)            = default;
    epoll_controller(epoll_controller &&)                 = delete;
    epoll_controller &operator=(const epoll_controller &) = default;
    epoll_controller &operator=(epoll_controller &&)      = delete;
    explicit epoll_controller(std::vector<epoll_event> m_events)
        : m_events(std::move(m_events)) { }
    ~epoll_controller();

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
