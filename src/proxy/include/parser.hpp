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
 * \param[in] host хост
 * \param[out] ip_address ссылка на адрес
 * \param[in] port ссылка на порт
 */
bool resolve_host(const std::string &host, std::string &ip_address, int &port);
