#pragma once


#include "context.hpp"

/*!
 * \brief Интерфейс для прокси сервера
 */
class proxy_server_iface {
public:
    virtual void add_client_socket(int fd) = 0;

    virtual void add_server_socket(int fd) = 0;

    virtual void change_sock_mod(int fd, uint32_t op) = 0;

    virtual void start_server_loop() = 0;

    virtual context_t *get_selector_ptr() = 0;

    virtual void add_new_connection(int fd, void *con) = 0;
};
