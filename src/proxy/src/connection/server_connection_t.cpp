#include "connection/server_connection_t.hpp"

#include <cstring>

#include <netdb.h>

#include <spdlog/spdlog.h>

#include "const.hpp"
#include "http.hpp"
#include "status_check.hpp"
#include "storage.hpp"

#include "proxy/proxy_runtime_exception.hpp"


server_connection_t::server_connection_t(
    const std::string &host,
    const std::string &request,
    std::pair<std::string, std::shared_ptr<item_t>> &storage_item,
    proxy_server_iface *server
)
    : m_fd(socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)),
      m_request_offset(0),
      m_storage_item(storage_item),
      m_content_len(-1),
      m_content_offset(0),
      m_http_code(0),
      m_stage(SERVER_STAGE_CONNECT),
      m_is_removed_due_to_unused(false),
      m_request_to_send(request),
      m_host(host) {
    addrinfo hints {};
    addrinfo *result;
    int s;
    std::string service = "http";
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = 0;
    hints.ai_protocol = 0;

    spdlog::debug(
        "server connection on sock_fd: {} for url{}", m_fd, m_storage_item.first
    );

    s = getaddrinfo(m_host.c_str(), service.data(), &hints, &result);
    if (s != 0) {
        throw std::runtime_error(strerror(errno));
    }

    int res = connect(m_fd, result->ai_addr, result->ai_addrlen);
    freeaddrinfo(result);
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
    spdlog::debug("close server connection sock_fd: {}", m_fd);
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

bool server_connection_t::check_usage() {
    if (m_storage_item.second->get_pin_count() == 0) {
        bool res = storage::instance().try_remove_if_unused(m_storage_item);
        m_is_removed_due_to_unused = res;
        return res;
    }
    return false;
}

bool server_connection_t::process_output(proxy_server_iface *server) {
    spdlog::trace("server_connection_t process output");
    if (check_usage()) {
        spdlog::debug("check_usage = true");
        return true;
    }

    if (m_stage == SERVER_STAGE_CONNECT) {
        spdlog::debug("stage CONNECT");
        sockaddr addr {};
        socklen_t len = sizeof(addr);
        int res       = getpeername(m_fd, &addr, &len);

        if (res == 0) {
            spdlog::debug("stage switched to SEND REQUEST");
            m_stage = SERVER_STAGE_SEND_REQUEST;
        }
        else {
            if (errno == ENOTCONN) {
                return false;
            }
            throw std::runtime_error("can not connect to server");
        }
    }

    if (m_stage != SERVER_STAGE_SEND_REQUEST) {
        spdlog::debug("stage != SERVER_STAGE_SEND_REQUEST");
        return false;
    }
    spdlog::trace("tring to write");

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
            return false;
        }
        throw proxy_runtime_exception(strerror(errno), errno);
    }

    m_request_offset += res;

    if (m_request_offset == m_request_to_send.length()) {
        m_request_to_send.clear();
        spdlog::debug("stage switched to READ FIRST LINE");
        m_stage = server_stages::SERVER_STAGE_READ_FIRST_LINE;
        server->change_sock_mod(m_fd, EPOLLIN);
    }
    return false;
}

bool server_connection_t::process_input(
    [[maybe_unused]] proxy_server_iface *server
) {
    spdlog::info("srever connection process_input");
    if (check_usage()) {
        return true;
    }

    if (m_stage != server_stages::SERVER_STAGE_READ_FIRST_LINE
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

        if (m_stage == server_stages::SERVER_STAGE_READ_FIRST_LINE
            || m_stage == server_stages::SERVER_STAGE_READ_HEADERS) {
            m_storage_item.second->put_data(m_tmp_answer_buffer);
        }
        throw proxy_runtime_exception(strerror(errno), errno);
    }

    if (m_stage == server_stages::SERVER_STAGE_READ_FIRST_LINE
        || m_stage == server_stages::SERVER_STAGE_READ_HEADERS) {
        m_tmp_answer_buffer.append(read_buffer.data(), res);
    }

    if (m_stage == server_stages::SERVER_STAGE_READ_FIRST_LINE) {
        size_t end_line_pos = m_tmp_answer_buffer.find("\r\n");
        if (end_line_pos == std::string::npos) {
            return false;
        }

        size_t first_space_pos = m_tmp_answer_buffer.find(' ');

        if (first_space_pos == std::string::npos
            || first_space_pos > end_line_pos) {
            throw proxy_runtime_exception("invalid answer format", 19);
        }

        std::string code_string =
            m_tmp_answer_buffer.substr(first_space_pos + 1, 3);

        m_http_code               = std::stoi(code_string);
        size_t size_before_change = m_tmp_answer_buffer.length();
        change_http_version_in_message(m_tmp_answer_buffer, 0, first_space_pos);
        size_t size_after_change = m_tmp_answer_buffer.length();
        m_stage                  = server_stages::SERVER_STAGE_READ_HEADERS;
        m_last_unparsed_line_start =
            end_line_pos + 2 + (size_after_change - size_before_change);

        spdlog::debug(
            "recieved http_code: {} on server connection sock_fd: {}",
            m_http_code,
            m_fd
        );
    }

    if (m_stage == server_stages::SERVER_STAGE_READ_HEADERS) {
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

            if (line.substr(0, param_end_index) == "Content-Length") {
                m_content_len = std::stol(line.substr(param_end_index + 1));
            }
            m_last_unparsed_line_start = end_line_pos + 2;
        }
    }


    if (m_stage == server_stages::SERVER_STAGE_READ_TILL_END) {
        size_t read_count = res;
        if (m_content_len > 0) {
            read_count = std::min(
                read_count,
                static_cast<size_t>(m_content_len) - m_content_offset
            );
        }
        std::string current_data(read_buffer.data(), read_count);

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
    }
    return false;
}
