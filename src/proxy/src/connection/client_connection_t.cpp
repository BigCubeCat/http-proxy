#include "connection/client_connection_t.hpp"

#include <stdexcept>

#include <spdlog/spdlog.h>

#include "const.hpp"
#include "http.hpp"
#include "status_check.hpp"
#include "storage.hpp"

#include "connection/server_connection_t.hpp"


client_connection_t::client_connection_t(int fd)
    : m_fd(fd),
      m_last_unparsed_line_start(0),
      m_stage(client_stages::CLIENT_STAGE_READ_FIRST_LINE),
      m_send_offset(0) { }

void client_connection_t::change_to_write_stage(proxy_server_iface *server) {
    spdlog::debug("client with sock_fd {} fully parsed request", m_fd);
    auto item = storage::instance().get_item(m_host + m_url);
    if (!item.second->is_started()) {
        bool res = item.second->set_started(true);
        if (!res) {
            auto *server_connection =
                new server_connection_t(m_host, m_request, item, server);
            server->add_new_connection(
                server_connection->get_fd(), server_connection
            );
        }
    }
    m_storage_item = item.second;
    server->change_sock_mod(m_fd, EPOLLOUT);
    m_stage = client_stages::CLIENT_STAGE_SEND_ANSWER;
}

bool client_connection_t::process_input(
    [[maybe_unused]] proxy_server_iface *server
) {
    if (m_stage == client_stages::CLIENT_STAGE_SEND_ANSWER) {
        return false;
    }

    std::array<char, BUFFER_SIZE> read_buffer {};
    auto res = read(m_fd, read_buffer.data(), BUFFER_SIZE);
    if (res == 0) {
        throw std::runtime_error("client closed socket");
    }
    if (res == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return false;
        }
        throw std::runtime_error(strerror(errno));
    }

    m_request.append(read_buffer.data(), res);
    if (m_stage == client_stages::CLIENT_STAGE_READ_FIRST_LINE) {
        size_t end_line_pos = m_request.find("\r\n");
        if (end_line_pos == std::string::npos) {
            return false;
        }
        if (m_request.substr(0, 3) != "GET") {
            spdlog::error("unsuportable protocol operation");
            return false;
        }
        size_t last_space_pos = m_request.find_last_of(' ', end_line_pos);
        if (last_space_pos == std::string::npos) {
            spdlog::error("invalid GET request header");
            return false;
        }
        change_http_version_in_message(
            m_request, last_space_pos + 1, end_line_pos - 1 - last_space_pos
        );
        m_url                      = m_request.substr(4, last_space_pos - 4);
        m_stage                    = client_stages::CLIENT_STAGE_READ_HOST;
        m_last_unparsed_line_start = end_line_pos + 2;
    }

    if (m_stage == client_stages::CLIENT_STAGE_READ_HOST) {
        while (m_last_unparsed_line_start < m_request.length()) {
            size_t end_line_pos =
                m_request.find("\r\n", m_last_unparsed_line_start);
            if (end_line_pos == std::string::npos) {
                return false;
            }

            if (m_last_unparsed_line_start == end_line_pos) {
                change_to_write_stage(server);
                return false;
            }

            std::string line = m_request.substr(
                m_last_unparsed_line_start,
                end_line_pos - m_last_unparsed_line_start
            );
            size_t param_end_index = line.find(':');
            if (param_end_index == std::string::npos) {
                spdlog::error("invalid parametr in HTTP header");
                return false;
            }
            if (line.substr(0, param_end_index) == "Host") {
                m_host                     = line.substr(param_end_index + 2);
                m_last_unparsed_line_start = end_line_pos + 2;
                m_stage = client_stages::CLIENT_STAGE_READ_TILL_END;
                break;
            }
            m_last_unparsed_line_start = end_line_pos + 2;
        }
    }
    if (m_stage == client_stages::CLIENT_STAGE_READ_TILL_END) {
        size_t end_pos = m_request.find_last_of("\r\n\r\n");
        if (end_pos != std::string::npos) {
            change_to_write_stage(server);
            return false;
        }
    }
    return false;
}


bool client_connection_t::process_output(
    [[maybe_unused]] proxy_server_iface *server
) {
    if (m_stage != client_stages::CLIENT_STAGE_SEND_ANSWER) {
        return false;
    }

    std::string buffer;

    int res = m_storage_item->get_data(
        buffer,
        m_send_offset,
        BUFFER_SIZE,
        wait_context_t(m_fd, server->get_selector_ptr())
    );
    if (res == -1) {
        return true;
    }

    if (res == 0) {    // we wait untill data arive in storage_item
        return false;
    }

    ssize_t send_res = write(m_fd, buffer.c_str(), buffer.length());
    if (send_res == -1) {
        if (errno == EAGAIN) {
            return false;
        }
        throw std::runtime_error(strerror(errno));
    }
    if (send_res == 0) {
        return true;
    }

    m_send_offset += static_cast<size_t>(send_res);
    return false;
}


client_connection_t::~client_connection_t() {
    if (m_storage_item) {
        m_storage_item->unpin(m_fd);
    }
    spdlog::debug("close client connection sock_fd: {}", m_fd);
    warn_status(close(m_fd), "close failed");
}
