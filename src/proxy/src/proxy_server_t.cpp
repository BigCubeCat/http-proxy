#include "proxy_server_t.hpp"

#include <spdlog/spdlog.h>

#include "connection/client_connection_t.hpp"
#include "proxy/proxy_runtime_exception.hpp"

proxy_server_t::proxy_server_t() = default;

void proxy_server_t::erase_connection(int fd, connection_t *connection) {
    m_connections.erase(fd);
    m_context.unregister_file_descriptor(fd);
    delete connection;
}


proxy_server_t::~proxy_server_t() {
    for (auto con : m_connections) {
        delete con.second;
    }
}


void proxy_server_t::start_server_loop() {
    while (true) {
        int n = m_context.do_select();

        for (int i = 0; i < n; ++i) {
            int fd                   = m_context[i].data.fd;
            uint32_t event           = m_context[i].events;
            auto it                  = m_connections.find(fd);
            connection_t *connection = nullptr;
            if (it == m_connections.end()) {
                connection = new client_connection_t(fd);
                auto res   = m_connections.emplace(fd, connection);
                if (!res.second) {
                    m_context.unregister_file_descriptor(fd);
                    delete connection;
                    continue;
                }
            }
            else {
                connection = it->second;
            }

            try {
                if (event & (EPOLLERR | EPOLLHUP)) {
                    erase_connection(fd, connection);
                    continue;
                }
                if (event & EPOLLIN) {
                    if (connection->process_input(this)) {
                        erase_connection(fd, connection);
                        continue;
                    }
                }
                if (event & EPOLLOUT) {
                    if (connection->process_output(this)) {
                        erase_connection(fd, connection);
                        continue;
                    }
                }
            }
            catch (const std::exception &e) {
                spdlog::critical("{}", e.what());
                erase_connection(fd, connection);
            }
        }
    }
}


void proxy_server_t::add_client_socket(int fd) {
    m_context.register_file_descriptor(fd, EPOLLIN);
}


void proxy_server_t::change_sock_mod(int fd, uint32_t op) {
    m_context.change_descriptor_mode(fd, op);
}


void proxy_server_t::add_server_socket(int fd) {
    m_context.register_file_descriptor(fd, EPOLLOUT);
}


void proxy_server_t::add_new_connection(int fd, void *con) {
    auto *connection_ptr = static_cast<connection_t *>(con);
    auto res             = m_connections.emplace(fd, connection_ptr);
    if (!res.second) {
        delete connection_ptr;
        throw proxy_runtime_exception("can't add new connection", 22);
    }
}
