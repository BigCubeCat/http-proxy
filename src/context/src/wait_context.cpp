#include "wait_context.hpp"

#include <spdlog/spdlog.h>

void wait_context_t::notify() noexcept {
    if (!m_context_ptr) {
        spdlog::critical("context pointer is nullptr");
    }
    m_context_ptr->change_descriptor_mode(m_fd, EPOLLOUT);
}
