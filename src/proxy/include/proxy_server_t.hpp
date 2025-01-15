#pragma once

#include <map>

#include "context.hpp"
#include "proxy_server_iface.hpp"

#include "connection/connection_t.hpp"

/*!
 * \brief сущность прокси-сервера, запускаемая в client worker
 */
class proxy_server_t : public proxy_server_iface {
    std::map<int, connection_t *> m_connections;
    context_t m_context;

    void erase_connection(int fd, connection_t *connection);

public:
    explicit proxy_server_t();

    proxy_server_t(int connections, int selector_context) { }

    ~proxy_server_t();

    void add_client_socket(int fd) override;

    void add_server_socket(int fd) override;

    void change_sock_mod(int fd, uint32_t op) override;
    void start_server_loop() override;

    context_t *get_selector_ptr() override {
        return &m_context;
    }

    void add_new_connection(int fd, void *con) override;
};
