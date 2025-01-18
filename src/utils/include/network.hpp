#pragma once

#include <string>

#include <netinet/in.h>

/*!
 * \brief Переключает файловый дискриптор в неблокирующий режим
 * \param[in] fd файловый дескриптор
 * \returns Возращает false в случае ошибки
 */
bool set_not_blocking(int fd);

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
int open_http_socket(const std::string &host, int &res);
