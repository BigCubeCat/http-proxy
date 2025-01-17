#pragma once


#include <memory>

#include "context.hpp"
#include "item.hpp"

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

    virtual void init_server_connection(
        const std::string &host,
        const std::string &request,
        std::pair<std::string, std::shared_ptr<item_t>> item
    ) = 0;
};
