#include "epoll_controller.hpp"

#include <stdexcept>

#include <sys/epoll.h>

#include "const.hpp"
#include "status_check.hpp"


epoll_controller::epoll_controller() : m_epoll_fd(epoll_create(MAX_EVENTS)) {
    if (m_epoll_fd < 0) {
        throw std::runtime_error("epoll_create failed");
    }
    m_events.resize(MAX_EVENTS);
    for (int i = 0; i < MAX_EVENTS; ++i) {
        m_events[i].data.fd = -1;
        m_events[i].events  = 0;
    }
}

epoll_controller::~epoll_controller() {
    warn_status(close(m_epoll_fd), "close epoll fd");
}


void epoll_controller::register_fd(int fd, uint32_t op) noexcept {
    m_events[fd].events  = 0;
    m_events[fd].data.fd = fd;
    if (op & EPOLLIN) {
        m_events[fd].events |= EPOLLIN;
    }
    if (op & EPOLLOUT) {
        m_events[fd].events |= EPOLLOUT;
    }
    error_status(
        epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &m_events[fd]), "epoll add"
    );
}

void epoll_controller::change_fd_mode(int fd, uint32_t op) noexcept {
    if (op & EPOLLIN) {
        m_events[fd].events |= EPOLLIN;
    }
    if (op & EPOLLOUT) {
        m_events[fd].events |= EPOLLOUT;
    }
    error_status(
        epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &m_events[fd]), "epoll change"
    );
}

void epoll_controller::unregister_fd(int fd) noexcept {
    warn_status(
        epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, &m_events[fd]), "epoll del"
    );
}

int epoll_controller::select() noexcept {
    return epoll_wait(
        m_epoll_fd, m_events.data(), MAX_EVENTS, EPOLL_WAIT_TIMEOUT
    );
}
