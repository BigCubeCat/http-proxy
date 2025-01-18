#include "connection/listen_connection.hpp"

#include <spdlog/spdlog.h>
#include <sys/socket.h>

#include "network.hpp"
#include "status_check.hpp"
#include "storage.hpp"


listen_connection_t::listen_connection_t(int listen_fd, thread_pool_t *pool_ptr)
    : m_fd(listen_fd), m_pool_ptr(pool_ptr) { }


int listen_connection_t::accept_client() {
    auto client_fd = accept(m_fd, nullptr, nullptr);
    if (client_fd < 0) {
        return client_fd;
    }
    if (!set_not_blocking(client_fd)) {
        return -2;
    }
    m_pool_ptr->add_task(client_fd);
    storage::instance().try_clean();
    return client_fd;
}

bool listen_connection_t::process_input(proxy_server_iface *server) {
    debug_status(accept_client(), "accept client status");
    return false;
}
