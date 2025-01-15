#pragma once

#include <memory>
#include <string>

#include "item.hpp"

#include "connection/connection_t.hpp"

enum server_stages {
    SERVER_STAGE_CONNECT,
    SERVER_STAGE_SEND_REQUEST,
    SERVER_STAGE_READ_FIRST_LINE,
    SERVER_STAGE_READ_HEADERS,
    SERVER_STAGE_READ_TILL_END
};

class server_connection_t : public connection_t {
    int m_fd;
    size_t m_request_offset;
    std::pair<std::string, std::shared_ptr<item_t>> m_storage_item;
    ssize_t m_content_len;
    size_t m_content_offset;
    size_t m_last_unparsed_line_start = 0;
    int m_http_code;
    server_stages m_stage;
    bool m_is_removed_due_to_unused;

    std::string m_request_to_send;
    std::string m_tmp_answer_buffer;
    std::string m_host;

    /*!
     * return true if item is removed from storage due pin_count = 0
     */
    bool check_usage();

public:
    server_connection_t(
        std::string &host,
        std::string &request,
        std::pair<std::string, std::shared_ptr<item_t>> &storage_item,
        proxy_server_iface *server
    );

    bool process_input([[maybe_unused]] proxy_server_iface *server) override;

    bool process_output([[maybe_unused]] proxy_server_iface *server) override;

    [[nodiscard]] int get_fd() const {
        return m_fd;
    }

    ~server_connection_t() override;
};
