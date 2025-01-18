#pragma once

#include "context.hpp"


class wait_context_t {
    int m_fd;

    context_t *m_context_ptr;

public:
    wait_context_t(int fd, context_t *sel_con)
        : m_fd(fd), m_context_ptr(sel_con) { }

    [[nodiscard]] int fd() const noexcept {
        return m_fd;
    }

    [[nodiscard]] context_t *get_context_ptr() const noexcept {
        return m_context_ptr;
    }

    void notify() noexcept;
};
