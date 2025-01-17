#pragma once

#include <memory>
#include <string>

#include "item.hpp"

#include "connection/connection_t.hpp"

enum server_stages {
    SERVER_STAGE_CONNECT         = 10,
    SERVER_STAGE_SEND_REQUEST    = 20,
    SERVER_STAGE_READ_FIRST_LINE = 30,
    SERVER_STAGE_READ_HEADERS    = 40,
    SERVER_STAGE_READ_TILL_END   = 50
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
     * \return true если запись не используется и мы можем начать сохранять в
     * кэш
     */
    bool check_usage();

    bool read_till_end(const std::array<char, BUFFER_SIZE> &buf, ssize_t res);

    bool check_connection();

    void write_part();

public:
    server_connection_t(
        const std::string &host,
        const std::string &request,
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
