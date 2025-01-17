#pragma once

#include <map>
#include <memory>

#include "context.hpp"
#include "proxy_server_iface.hpp"
#include "thread_pool.hpp"

#include "connection/connection_t.hpp"

/*!
 * \brief сущность прокси-сервера, запускаемая в client worker
 */
class proxy_server_t : public proxy_server_iface {
private:
    bool m_is_running = true;
    std::map<int, std::shared_ptr<connection_t>> m_connections;
    context_t m_context;

    void erase_connection(int fd);

public:
    ~proxy_server_t() = default;
    explicit proxy_server_t();

    proxy_server_t(int connections, int selector_context) { }

    void add_client_socket(int fd) override;

    void add_server_socket(int fd) override;

    void change_sock_mod(int fd, uint32_t op) override;
    void start_server_loop() override;

    context_t *get_selector_ptr() override {
        return &m_context;
    }

    void init_server_connection(
        const std::string &host,
        const std::string &request,
        std::pair<std::string, std::shared_ptr<item_t>> item
    ) override;

    void init_listen_connection(thread_pool_t *pool_ptr, int fd);

    void stop() {
        m_is_running = false;
    };
};
