#pragma once

#include "connection_t.hpp"
#include "thread_pool.hpp"

/*!
 * Интерфейс процессинга соединения
 */
class listen_connection_t : public connection_t {
private:
    int m_fd;
    thread_pool_t *m_pool_ptr;

    int accept_client();

public:
    listen_connection_t(int listen_fd, thread_pool_t *pool_ptr);

    bool process_input([[maybe_unused]] proxy_server_iface *server) override;

    bool process_output([[maybe_unused]] proxy_server_iface *server) override {
        return false;
    };

    ~listen_connection_t() override = default;
};
