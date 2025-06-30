#include "utils.hpp"

#include <csignal>
#include <regex>
#include <sstream>

#include "status_check.hpp"


using std::regex;

void parse_http_header(
    const std::string &response,
    std::unordered_map<std::string, std::string> &headers,
    int &status
) {
    std::istringstream stream(response);
    std::string line;

    if (std::getline(stream, line)) {
        std::istringstream status_line(line);
        std::string http_version;
        status_line >> http_version;
        status_line >> status;
    }
    while (std::getline(stream, line) && line != "\r") {
        auto colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key   = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            // Удаляем ведущие и завершающие пробелы
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            headers[key] = value;
        }
    }
}

bool is_url(const std::string &possible_url) {
    static regex url_regex(
        R"(^https?:\/\/(?:localhost|\b\d{1,3}(\.\d{1,3}){3}\b|(?:[a-zA-Z0-9\-]+\.)+[a-zA-Z]{2,})(:\d+)?(\/[^\s]*)?$)"
    );
    return std::regex_match(possible_url, url_regex);
}

bool is_localhost_url(const std::string &possible_localhost) {
    auto localhost_pos = possible_localhost.find("localhost");
    if (localhost_pos != std::string::npos) {
        return true;
    }
    localhost_pos = possible_localhost.find("127.0.0.1");
    return (localhost_pos != std::string::npos);
}

void disable_signals() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGPIPE);
    error_status(
        pthread_sigmask(SIG_BLOCK, &set, nullptr), "pthread_sigmask failed"
    );
}

int current_time() {
    const auto p1 = std::chrono::system_clock::now();
    return static_cast<int>(
        std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch())
            .count()
        % 1000000
    );
}
