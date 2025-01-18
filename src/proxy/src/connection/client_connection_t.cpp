#include "connection/client_connection_t.hpp"

#include <stdexcept>

#include <spdlog/spdlog.h>

#include "const.hpp"
#include "parser.hpp"
#include "status_check.hpp"
#include "storage.hpp"


client_connection_t::client_connection_t(int fd)
    : m_fd(fd),
      m_last_unparsed_line_start(0),
      m_stage(client_stages::CLIENT_STAGE_READ_REQUEST),
      m_send_offset(0) { }

void client_connection_t::change_to_write_stage(proxy_server_iface *server) {
    spdlog::debug("client with sock_fd {} fully parsed request", m_fd);
    auto item = storage::instance().get_item(m_host + m_url);
    if (!item.second->is_started()) {
        spdlog::trace("item not started");
        bool res = item.second->set_started(true);
        if (!res) {
            spdlog::trace("create server connection");
            server->init_server_connection(m_host, m_request, item);
            spdlog::trace("adding new connection");
        }
    }
    spdlog::trace("storage item loaded");
    m_storage_item = item.second;
    server->change_sock_mod(m_fd, EPOLLOUT);
    spdlog::debug("stage switched to SEND ANSWER");
    m_stage = client_stages::CLIENT_STAGE_SEND_ANSWER;
}

bool client_connection_t::process_input(
    [[maybe_unused]] proxy_server_iface *server
) {
    spdlog::debug("current stage is {}", static_cast<int>(m_stage));
    if (m_stage == client_stages::CLIENT_STAGE_SEND_ANSWER) {
        spdlog::debug("stage SEND ANSWER");
        return false;
    }
    std::array<char, BUFFER_SIZE> read_buffer {};
    auto res = read(m_fd, read_buffer.data(), BUFFER_SIZE);
    if (res == 0) {
        throw std::runtime_error("client closed socket");
    }
    if (res == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            spdlog::warn("client_connection_t process input: EAGAIN recv");
            return false;
        }
        spdlog::error("client connection cant read");
        throw std::runtime_error(strerror(errno));
    }

    m_request.append(read_buffer.data(), res);
    spdlog::trace("request = {}", m_request);
    if (m_stage == client_stages::CLIENT_STAGE_READ_REQUEST) {
        spdlog::debug("stage READ FIRST LINE");
        // проверяем, закончено ли получение запроса
        size_t end_line_pos = m_request.find("\r\n\r\n");
        if (end_line_pos == std::string::npos) {
            spdlog::debug("here");
            return false;
        }
        std::string method;
        if (!parse_request(m_request, method, m_url, m_host)) {
            spdlog::error("invalid client request; {}", m_request);
            return false;
        }
        if (method != "GET") {
            spdlog::error("unsuportable protocol operation");
            return false;
        }
        change_to_write_stage(server);
    }
    return false;
}


bool client_connection_t::process_output(
    [[maybe_unused]] proxy_server_iface *server
) {
    if (m_stage != client_stages::CLIENT_STAGE_SEND_ANSWER) {
        spdlog::debug("client_connection_t: stage != CLIENT_STAGE_SEND_ANSWER");
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
        spdlog::error("client_connection_t: invalid write {}", m_fd);
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
