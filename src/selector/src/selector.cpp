#include "selector.hpp"

#include <spdlog/spdlog.h>

#include "exceptions.hpp"

selector_t::selector_t(int count_events) : m_count_events(count_events) {
    m_epoll_fd = epoll_create(m_count_events);
    m_events.resize(m_count_events);

    if (m_epoll_fd == -1) {
        perror("creating epoll");
        abort();
    }
    for (int i = 0; i < m_count_events; i++) {
        m_events[i].events  = 0;
        m_events[i].data.fd = -1;
    }
}

void selector_t::selector_epoll_ctl(int fd, int operation, uint32_t op) {
    epoll_event event {};
    event.data.fd = fd;
    event.events  = 0;
    if (op & EPOLLIN) {
        event.events |= EPOLLIN;
    }
    if (op & EPOLLOUT) {
        event.events |= EPOLLOUT;
    }
    auto status = epoll_ctl(m_epoll_fd, operation, fd, &event);
    if (status != 0) {
        throw selector_exception(
            "error on epoll_ctl; status=" + std::to_string(status)
        );
    }
}


selector_t::~selector_t() {
    auto status = close(m_epoll_fd);
    if (status != 0) {
        spdlog::critical(
            "close selector_t epoll_fd failed with status {}", m_epoll_fd
        );
    }
}

void selector_t::append_fd(int fd, uint32_t op) {
    selector_epoll_ctl(fd, EPOLL_CTL_ADD, op);
}

void selector_t::update_fd(int fd, uint32_t op) {
    selector_epoll_ctl(fd, EPOLL_CTL_MOD, op);
}

void selector_t::delete_fd(int fd) {
    auto status = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    if (status != 0) {
        throw selector_exception(
            "error on epoll_ctl (delete_fd); status=" + std::to_string(status)
        );
    }
}

int selector_t::select() {
    return epoll_wait(m_epoll_fd, m_events.data(), m_count_events, -1);
}
