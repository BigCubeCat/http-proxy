#pragma once

#include <string>


/*!
 * \brief Задаем версию HTTP 1.0
 */
void change_http_version_in_message(
    std::string &request, size_t http_ver_index, size_t http_ver_size
);
