#pragma once


#include <string>
#include <unordered_map>


void parse_http_header(
    const std::string &response,
    std::unordered_map<std::string, std::string> &headers,
    int &status
);

/*!
 * \brief Вернет true, если переданная строка является url
 */
bool is_url(const std::string &possible_url);

/*!
 * \brief Вернет true, если переданный url является локальным
 */
bool is_localhost_url(const std::string &possible_localhost);

/*!
 * \brief Ставит маску сигнала в блокировку всех сигналов
 * Вызывать эту функцию только в поточной функции
 */
void disable_signals();


/*!
 * \brief возращает текущее время время
 */
int current_time();
