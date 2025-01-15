#pragma once

#include <memory>
#include <string>

#include "item.hpp"

#include "connection/connection_t.hpp"

enum client_stages {
    CLIENT_STAGE_READ_FIRST_LINE,
    CLIENT_STAGE_READ_HOST,
    CLIENT_STAGE_READ_TILL_END,
    CLIENT_STAGE_SEND_ANSWER
};

class client_connection_t : public connection_t {
    int m_fd;
    std::string m_request;
    std::string m_host;
    std::string m_url;
    std::shared_ptr<item_t> m_storage_item;
    size_t m_last_unparsed_line_start;
    client_stages m_stage;
    size_t m_send_offset;

    void change_to_write_stage(proxy_server_iface *server);

public:
    client_connection_t(int fd);

    bool process_input([[maybe_unused]] proxy_server_iface *server) override;

    bool process_output([[maybe_unused]] proxy_server_iface *server) override;

    ~client_connection_t() override;
};
