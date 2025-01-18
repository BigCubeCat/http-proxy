#pragma once

#include <exception>
#include <string>

class proxy_runtime_exception : public std::exception {
private:
    std::string m_message;

public:
    explicit proxy_runtime_exception(const std::string &msg, int status)
        : m_message(
              "proxy runtime exception: " + msg
              + "; status=" + std::to_string(status)
          ) { };

    [[nodiscard]] const char *what() const noexcept override {
        return m_message.c_str();
    };
};
