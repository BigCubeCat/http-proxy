#pragma once

#include <string>

#include <spdlog/spdlog.h>


/*!
 * \brief Записывает в лог статус, если он не нулевой
 */
template <typename T>
void debug_status(T fd, const std::string &message) {
    if (fd != 0) {
        spdlog::debug("{}: {}", message, fd);
    }
}


/*!
 * \brief Записывает в warning статус, если он не нулевой
 */
template <typename T>
void warn_status(T fd, const std::string &message) {
    if (fd != 0) {
        spdlog::warn("{}: {}", message, fd);
    }
}

/*!
 * \brief Записывает в error статус, если он не нулевой
 */
template <typename T>
bool error_status(T fd, const std::string &message) {
    if (fd < 0) {
        spdlog::error("{}: {}", message, fd);
        return false;
    }
    return true;
}
