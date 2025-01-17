#include "proxy_server_t.hpp"

#include <spdlog/spdlog.h>

#include "connection/client_connection_t.hpp"
#include "connection/listen_connection.hpp"
#include "connection/server_connection_t.hpp"
#include "proxy/proxy_runtime_exception.hpp"

proxy_server_t::proxy_server_t() = default;

void proxy_server_t::erase_connection(int fd) {
    m_connections.erase(fd);
    m_context.unregister_file_descriptor(fd);
}

void proxy_server_t::start_server_loop() {
    while (m_is_running) {
        spdlog::trace("do select");
        int n = m_context.do_select();

        for (int i = 0; i < n; ++i) {
            auto fd = m_context[i].data.fd;
            spdlog::trace("processing fd={}", fd);
            auto event = m_context[i].events;
            auto it    = m_connections.find(fd);
            std::shared_ptr<connection_t> connection;
            if (it == m_connections.end()) {
                spdlog::debug("connection not opened");
                connection = std::make_shared<client_connection_t>(fd);
                auto res   = m_connections.emplace(fd, connection);
                if (!res.second) {
                    spdlog::debug("cant open connection");
                    m_context.unregister_file_descriptor(fd);
                    connection.reset();
                    continue;
                }
            }
            else {
                connection = it->second;
            }

            try {
                if (event & (EPOLLERR | EPOLLHUP)) {
                    spdlog::trace("EPOLL ERROR");
                    erase_connection(fd);
                    continue;
                }
                if (event & EPOLLIN) {
                    spdlog::trace("EPOLLIN");
                    if (connection->process_input(this)) {
                        erase_connection(fd);
                        continue;
                    }
                }
                if (event & EPOLLOUT) {
                    spdlog::trace("EPOLLOUT");
                    if (connection->process_output(this)) {
                        spdlog::warn("process process_output");
                        erase_connection(fd);
                        continue;
                    }
                }
            }
            catch (const std::exception &e) {
                spdlog::critical("{}", e.what());
                erase_connection(fd);
            }
        }
    }
}

void proxy_server_t::add_client_socket(int fd) {
    spdlog::debug("add client socket {}", fd);
    m_context.register_file_descriptor(fd, EPOLLIN);
}

void proxy_server_t::change_sock_mod(int fd, uint32_t op) {
    spdlog::debug("change socket mode {}", fd);
    m_context.change_descriptor_mode(fd, op);
}

void proxy_server_t::add_server_socket(int fd) {
    spdlog::debug("add server socket {}", fd);
    m_context.register_file_descriptor(fd, EPOLLOUT);
}

void proxy_server_t::init_server_connection(
    const std::string &host,
    const std::string &request,
    std::pair<std::string, std::shared_ptr<item_t>> item
) {
    auto server_connection =
        std::make_shared<server_connection_t>(host, request, item, this);
    auto res =
        m_connections.emplace(server_connection->get_fd(), server_connection);
    if (!res.second) {
        throw proxy_runtime_exception("can't add new connection", 22);
    }
    add_server_socket(server_connection->get_fd());
}

void proxy_server_t::init_listen_connection(thread_pool_t *pool_ptr, int fd) {
    auto accept_connection =
        std::make_shared<listen_connection_t>(fd, pool_ptr);
    auto res = m_connections.emplace(fd, accept_connection);
    if (!res.second) {
        throw proxy_runtime_exception("can't add listen connection", 20);
    }
    add_client_socket(fd);
}
