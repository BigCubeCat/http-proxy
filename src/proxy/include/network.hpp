#pragma once

#include <string>

/*!
 * \brief Функция для отправки строки по сети
 * \param[in] socket файловый дискриптор
 * \param[in] data данные
 */
bool send_all(int socket, const std::string &data);

/*!
 * \brief Функция для получения данных из сети
 * \param[in] socket файловый дискриптор
 * \param[in] response ссылка на stream
 */
void recv_all(int socket, std::ostringstream &response);
