#pragma once

#include <string>

/*!
 * \brief Парсинг запроса с помощью picohttpparser
 */
bool parse_request(
    const char *buf,
    ssize_t len,
    std::string &method,
    std::string &url,
    std::string &host
);

/*!
 * \brief Резолвинг хоста (DNS)
 */
bool resolve_host(const std::string &host, std::string &ip_address);
