#include "context.hpp"

#include <unistd.h>

#include <spdlog/spdlog.h>

#include "status_check.hpp"


context_t::context_t() : m_ep_fd(epoll_create(MAX_EVENTS)) {
    m_events.resize(MAX_EVENTS);
    if (m_ep_fd == -1) {
        perror("creating epoll");
        abort();
    }
    for (int i = 0; i < MAX_EVENTS; i++) {
        m_events[i].events  = 0;
        m_events[i].data.fd = -1;
    }
}

context_t::~context_t() {
    error_status(close(m_ep_fd), "close epoll fd failed");
}

void context_t::register_file_descriptor(int fd, uint32_t op) const noexcept {
    spdlog::trace("register_file_descriptor {} {}", fd, op);
    epoll_event event {};
    event.data.fd = fd;
    event.events  = 0;
    if (op & EPOLLIN) {
        spdlog::trace("EPOLLIN");
        event.events |= EPOLLIN;
    }
    if (op & EPOLLOUT) {
        spdlog::trace("EPOLLOUT");
        event.events |= EPOLLOUT;
    }
    debug_status(
        epoll_ctl(m_ep_fd, EPOLL_CTL_ADD, fd, &event),
        // add
        "epoll_ctl failed"
    );
}

void context_t::change_descriptor_mode(int fd, uint32_t op) const noexcept {
    epoll_event event {};
    event.data.fd = fd;
    event.events  = 0;
    if (op & EPOLLIN) {
        event.events |= EPOLLIN;
    }
    if (op & EPOLLOUT) {
        event.events |= EPOLLOUT;
    }
    debug_status(
        epoll_ctl(m_ep_fd, EPOLL_CTL_MOD, fd, &event),
        // modification
        "epoll_ctl failed"
    );
}

void context_t::unregister_file_descriptor(int fd) const noexcept {
    debug_status(
        epoll_ctl(m_ep_fd, EPOLL_CTL_DEL, fd, nullptr),
        // delete
        "epoll_ctl del"
    );
}

int context_t::do_select() noexcept {
    return epoll_wait(m_ep_fd, m_events.data(), MAX_EVENTS, EPOLL_WAIT_TIMEOUT);
}
