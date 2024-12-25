#pragma once


#include <string>
#include <unordered_map>


void parse_http_header(
    const std::string &response,
    std::unordered_map<std::string, std::string> &headers,
    int &status
);
