#pragma once

#include <exception>
#include <string>

class selector_exception : public std::exception {
private:
    std::string m_message;

public:
    explicit selector_exception(const std::string &msg)
        : m_message("selector exception: " + msg) { };

    [[nodiscard]] const char *what() const noexcept override {
        return m_message.c_str();
    };
};
