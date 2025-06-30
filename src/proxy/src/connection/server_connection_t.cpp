#include "connection/server_connection_t.hpp"

#include <cstring>
#include <stdexcept>

#include <netdb.h>

#include <spdlog/spdlog.h>
#include <sys/types.h>

#include "const.hpp"
#include "http.hpp"
#include "network.hpp"
#include "status_check.hpp"
#include "storage.hpp"

#include "proxy/proxy_runtime_exception.hpp"


server_connection_t::server_connection_t(
    const std::string &host,
    const std::string &request,
    std::pair<std::string, std::shared_ptr<item_t>> &storage_item,
    proxy_server_iface *server
)
    : m_fd(-1),
      m_request_offset(0),
      m_storage_item(storage_item),
      m_content_len(-1),
      m_content_offset(0),
      m_http_code(0),
      m_stage(SERVER_STAGE_CONNECT),
      m_is_removed_due_to_unused(false),
      m_request_to_send(request),
      m_host(host) {
    spdlog::debug(
        "server connection on sock_fd: {} for url{}", m_fd, m_storage_item.first
    );
    int res;
    m_fd = open_http_socket(m_host, res);
    if (res == 0) {
        spdlog::debug("stage SEND REQUEST");
        m_stage = SERVER_STAGE_SEND_REQUEST;
        server->add_server_socket(m_fd);
        return;
    }
    if (errno == EINPROGRESS) {
        server->add_server_socket(m_fd);
        return;
    }
    warn_status(close(m_fd), "close fd error");
}

server_connection_t::~server_connection_t() {
    warn_status(close(m_fd), "close fd error");
    m_storage_item.second->set_completed(true);

    if (m_http_code != 200 || m_is_removed_due_to_unused) {
        storage::instance().remove_item(m_storage_item.first);
        spdlog::debug(
            "remove item from storage for key: {}", m_storage_item.first
        );
        return;
    }
    if (m_content_len > 0
        && static_cast<size_t>(m_content_len) != m_content_offset) {
        storage::instance().remove_item(m_storage_item.first);
        spdlog::debug(
            "did not recieve full content from server expected: {} get: {}",
            m_content_len,
            m_content_offset

        );
    }
}

bool server_connection_t::process_output(proxy_server_iface *server) {
    spdlog::trace("server_connection_t process output");
    if (check_usage()) {
        spdlog::debug("check_usage = true");
        return true;
    }

    if (m_stage == SERVER_STAGE_CONNECT) {
        // проверяем подключение
        if (!check_connection()) {
            return false;
        }
    }

    if (m_stage != SERVER_STAGE_SEND_REQUEST) {
        spdlog::debug("stage != SERVER_STAGE_SEND_REQUEST");
        return false;
    }

    write_part();

    if (m_request_offset == m_request_to_send.length()) {
        m_request_to_send.clear();
        spdlog::debug("stage switched to READ FIRST LINE");
        m_stage = server_stages::SERVER_STAGE_READ_HEADERS;
        server->change_sock_mod(m_fd, EPOLLIN);
    }

    return false;
}

bool server_connection_t::process_input(
    [[maybe_unused]] proxy_server_iface *server
) {
    if (check_usage()) {
        return true;
    }
    if (m_stage != server_stages::SERVER_STAGE_READ_HEADERS
        && m_stage != server_stages::SERVER_STAGE_READ_TILL_END
        && m_stage != server_stages::SERVER_STAGE_READ_HEADERS) {
        return false;
    }

    std::array<char, BUFFER_SIZE> read_buffer {};
    ssize_t res = read(m_fd, read_buffer.data(), BUFFER_SIZE);
    if (res == 0) {
        if (m_stage != server_stages::SERVER_STAGE_READ_TILL_END) {
            m_storage_item.second->put_data(m_tmp_answer_buffer);
        }
        return true;
    }
    if (res == -1) {
        if (errno == EAGAIN) {
            return false;
        }
        if (m_stage == server_stages::SERVER_STAGE_READ_HEADERS) {
            m_storage_item.second->put_data(m_tmp_answer_buffer);
        }
        spdlog::error("server_connection_t: invalid read");
        throw proxy_runtime_exception(strerror(errno), errno);
    }
    if (m_stage == server_stages::SERVER_STAGE_READ_HEADERS) {
        m_tmp_answer_buffer.append(read_buffer.data(), res);
        size_t end_line_pos = m_tmp_answer_buffer.find("\r\n");
        if (end_line_pos == std::string::npos) {
            return false;
        }

        size_t first_space_pos = m_tmp_answer_buffer.find(' ');

        if (first_space_pos == std::string::npos
            || first_space_pos > end_line_pos) {
            throw proxy_runtime_exception("invalid answer format", 19);
        }

        auto code_string = m_tmp_answer_buffer.substr(first_space_pos + 1, 3);
        m_http_code      = std::stoi(code_string);
        size_t size_before_change = m_tmp_answer_buffer.length();
        change_http_version_in_message(m_tmp_answer_buffer, 0, first_space_pos);
        size_t size_after_change = m_tmp_answer_buffer.length();
        m_last_unparsed_line_start =
            end_line_pos + 2 + (size_after_change - size_before_change);

        spdlog::debug(
            "recieved http_code: {} on server connection sock_fd: {}",
            m_http_code,
            m_fd
        );
        if (!read_headers()) {
            return false;
        }
    }

    if (m_stage == server_stages::SERVER_STAGE_READ_TILL_END) {
        return read_till_end(read_buffer, res);
    }
    return false;
}

bool server_connection_t::read_till_end(
    const std::array<char, BUFFER_SIZE> &buf, ssize_t res
) {
    size_t read_count = res;
    if (m_content_len > 0) {
        read_count = std::min(
            read_count, static_cast<size_t>(m_content_len) - m_content_offset
        );
    }
    std::string current_data(buf.data(), read_count);
    m_content_offset += read_count;
    m_storage_item.second->put_data(current_data);

    if (static_cast<ssize_t>(m_content_offset) == m_content_len) {
        spdlog::debug(
            "data from server is fully obtained on server connection "
            "sock_fd: {}",
            m_fd
        );
        m_storage_item.second->set_completed(true);
        return true;
    }
    return false;
}

bool server_connection_t::check_connection() {
    sockaddr addr {};
    socklen_t len = sizeof(addr);
    int res       = getpeername(m_fd, &addr, &len);

    if (res == 0) {
        spdlog::debug("stage switched to SEND REQUEST");
        m_stage = SERVER_STAGE_SEND_REQUEST;
        return true;
    }
    if (errno == ENOTCONN) {
        return false;
    }
    throw std::runtime_error("can not connect to server");
}

void server_connection_t::write_part() {
    ssize_t res = write(
        m_fd,
        m_request_to_send.c_str() + m_request_offset,
        std::min(
            m_request_to_send.length() - m_request_offset,
            static_cast<unsigned long>(BUFFER_SIZE)
        )
    );

    if (res == 0) {
        throw proxy_runtime_exception("connection is closed by the server", 17);
    }

    if (res == -1) {
        if (errno == EAGAIN) {
            spdlog::trace("server_connection_t: EAGAIN recv");
            return;
        }
        spdlog::error("server_connection_t: invalid write_part()");
        throw proxy_runtime_exception(strerror(errno), errno);
    }

    m_request_offset += res;
}

bool server_connection_t::read_headers() {
    while (m_last_unparsed_line_start < m_tmp_answer_buffer.length()) {
        size_t end_line_pos =
            m_tmp_answer_buffer.find("\r\n", m_last_unparsed_line_start);
        if (end_line_pos == std::string::npos) {
            return false;
        }

        if (m_last_unparsed_line_start == end_line_pos) {
            m_stage = server_stages::SERVER_STAGE_READ_TILL_END;
            m_storage_item.second->put_data(m_tmp_answer_buffer);
            m_content_offset =
                m_tmp_answer_buffer.length() - (end_line_pos + 2);
            m_tmp_answer_buffer.clear();
            return false;
        }

        std::string line = m_tmp_answer_buffer.substr(
            m_last_unparsed_line_start,
            end_line_pos - m_last_unparsed_line_start
        );
        size_t param_end_index = line.find(':');
        if (param_end_index == std::string::npos) {
            throw proxy_runtime_exception(
                "invalid parametr in HTTP header", 27
            );
        }

        auto key = line.substr(0, param_end_index);
        if (key == "Content-Length") {
            m_content_len = std::stol(line.substr(param_end_index + 1));
            spdlog::debug("Content-Length: {}", m_content_len);
        }
        else if (key == "Location") {
            m_location = line.substr(param_end_index + 1);
            spdlog::info("Location: {}", m_location);
            return false;
        }
        m_last_unparsed_line_start = end_line_pos + 2;
    }
    return true;
}

bool server_connection_t::check_usage() {
    return m_storage_item.second->get_pin_count() == 0;
}
