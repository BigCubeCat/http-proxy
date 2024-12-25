#include "http_response_parser.hpp"

#include <spdlog/spdlog.h>
#include <sys/socket.h>

#include "network.hpp"
#include "parser.hpp"
#include "status_check.hpp"
#include "utils.hpp"


http_response_processor_t::http_response_processor_t(int fd)
    : m_client_fd(fd) { }

bool http_response_processor_t::receive() {
    int status = 0;
    int sock_fd;
    std::unordered_map<std::string, std::string> headers_map;

    for (int steps = 0; status < HTTP_ERROR_STATUS && steps < MAXIMUM_MOVE;
         ++steps) {
        spdlog::debug("host = {}", m_host);
        sock_fd = open_http_socket(m_host);
        if (sock_fd < 0) {
            return false;
        }
        spdlog::debug("sock_fd = {}", sock_fd);
        debug_status(
            send(sock_fd, m_request.c_str(), m_request.size(), 0),
            "send request"
        );
        recv_all(sock_fd);
        parse_http_header(m_data, headers_map, status);
        debug_status(close(sock_fd), "close error");
        if (status / 100 != 3) {
            break;
        }
        auto location_value = headers_map.find("Location");
        if (location_value == headers_map.end()) {
            spdlog::error("invalid HTTP 300 response: no localtion header");
            return false;
        }
        m_host = location_value->second;
    }
    return true;
}

bool http_response_processor_t::process() {
    if (!m_data.empty()) {
        if (!send_all()) {
            spdlog::error("cant send");
        }
    }
    debug_status(close(m_client_fd), "close");
    return true;
}

bool http_response_processor_t::parse_client_request() {
    auto bytes_read = read(m_client_fd, m_buffer.data(), BUFFER_SIZE);
    if (bytes_read < 0) {
        return false;
    }
    if (!parse_request(m_buffer.data(), bytes_read, m_method, m_url, m_host)) {
        return false;
    }
    m_request = m_buffer.data();
    return true;
}

std::string http_response_processor_t::data() const {
    return m_data;
}

std::string http_response_processor_t::url() const {
    return m_url;
}

void http_response_processor_t::set_cached_data(std::string data) {
    m_data = std::move(data);
}

bool http_response_processor_t::send_all() {
    size_t total_bytes_sent = 0;
    size_t data_size        = m_data.size();
    size_t bytes_left;
    ssize_t bytes_sent;
    while (total_bytes_sent < data_size) {
        bytes_left = data_size - total_bytes_sent;
        bytes_sent = send(
            m_client_fd,
            m_data.data() + total_bytes_sent,
            BUFFER_SIZE > bytes_left ? bytes_left : BUFFER_SIZE,
            0
        );
        if (bytes_sent < 0) {
            spdlog::error("error sending data: {} {}", strerror(errno), errno);
            return false;
        }
        total_bytes_sent += static_cast<size_t>(bytes_sent);
    }
    return true;
}

void http_response_processor_t::recv_all(int fd) {
    std::ostringstream response;
    ssize_t bytes_read = recv(fd, m_buffer.data(), BUFFER_SIZE, 0);
    while (bytes_read > 0) {
        response.write(m_buffer.data(), bytes_read);
        bytes_read = recv(fd, m_buffer.data(), BUFFER_SIZE, 0);
    }
    m_data = response.str();
}
