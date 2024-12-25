#include "utils.hpp"

#include <sstream>


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
