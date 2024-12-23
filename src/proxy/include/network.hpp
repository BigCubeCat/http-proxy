#pragma once

#include <string>

#include <netinet/in.h>

/*!
 * \brief Функция для отправки строки по сети
 * \param[in] socket файловый дискриптор
 * \param[in] data данные
 */
bool send_all(int socket, const std::string &data);

/*!
 * \brief Функция для получения данных из сети
 * \param[in] socket файловый дискриптор
 * \returns полученные данные
 */
std::string recv_all(int socket);

/*!
 * \brief Переключает файловый дискриптор в неблокирующий режим
 * \param[in] fd файловый дескриптор
 */
void set_not_blocking(int fd);

/*!
 * \brief создание принимающего сокета
 * \param[in] fd listen fd
 * \param[in] port port
 */
int bind_socket(int fd, int port);

bool register_fd(int epoll_fd, int fd);

/*!
 * Производит подключение к http серверу, для чтения данных.
 * Возращает fd открытого сокета
 * В случае ошибки логирует сообщение об ошибке и возращает -1
 */
int open_http_socket(const std::string &host);
