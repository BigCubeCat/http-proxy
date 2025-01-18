#pragma once

#include <exception>
#include <string>

class parsing_exception : public std::exception {
private:
    std::string m_message;

public:
    explicit parsing_exception(const std::string &msg, int status)
        : m_message(
              "parsing_exception: " + msg + "; status=" + std::to_string(status)
          ) { };

    [[nodiscard]] const char *what() const noexcept override {
        return m_message.c_str();
    };
};
