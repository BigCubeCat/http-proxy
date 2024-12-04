#pragma once

#include <exception>
#include <string>

class thread_pool_exception : public std::exception {
private:
    std::string m_message;

public:
    explicit thread_pool_exception(const std::string &msg)
        : m_message("thread pool: " + msg) { };

    [[nodiscard]] const char *what() const noexcept override {
        return m_message.c_str();
    };
};
