#include "network.hpp"

#include <fcntl.h>

#include <arpa/inet.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "const.hpp"
#include "parser.hpp"
#include "status_check.hpp"

#include "proxy/proxy_runtime_exception.hpp"

bool send_all(int socket, const std::string &data) {
    size_t total_bytes_sent = 0;
    size_t data_size        = data.size();

    spdlog::debug("data size = {}", data_size);
    size_t bytes_left;
    ssize_t bytes_sent;
    while (total_bytes_sent < data_size) {
        bytes_left = data_size - total_bytes_sent;
        bytes_sent = send(
            socket,
            data.data() + total_bytes_sent,
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

void recv_all(int socket, std::ostringstream &response) {
    std::array<char, BUFFER_SIZE> recv_buffer {};
    ssize_t bytes_read = recv(socket, recv_buffer.data(), BUFFER_SIZE, 0);
    spdlog::trace("bytes_read = {}", bytes_read);
    while (bytes_read > 0) {
        response.write(recv_buffer.data(), bytes_read);
        bytes_read = recv(socket, recv_buffer.data(), BUFFER_SIZE, 0);
    }
}

void set_not_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, SO_REUSEPORT);
    if (flags == -1) {
        spdlog::critical("fcntl F_GETFL");
        throw proxy_runtime_exception("fcntl F_GETFL", -1);
    }
    flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (flags == -1) {
        spdlog::critical("fcntl F_SETFL");
        throw proxy_runtime_exception("fcntl F_SETFL", -1);
    }
}

int bind_socket(int fd, int port) {
    sockaddr_in server_addr {};
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(port);
    return bind(
        fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)
    );
}

bool register_fd(int epoll_fd, int fd) {
    epoll_event ev {};
    ev.events  = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    return error_status(
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev), "epoll_ctl ADD"
    );
}

int open_http_socket(const std::string &host) {
    std::string ip_address;
    if (!resolve_host(host, ip_address)) {
        spdlog::error("failed to resolve host: {}", host);
        return -1;
    }

    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(80);
    int sock_fd            = socket(AF_INET, SOCK_STREAM, 0);
    if (!error_status(sock_fd, "socket create failed")) {
        return -1;
    }
    auto inet_st =
        inet_pton(AF_INET, ip_address.c_str(), &server_addr.sin_addr);
    if (!error_status(inet_st, "inet pton error")) {
        warn_status(close(sock_fd), "close");
        return -1;
    }
    auto conn_st = connect(
        sock_fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)
    );
    warn_status(conn_st, "connection to server failed");
    if (conn_st < 0) {
        debug_status(close(sock_fd), "close socket failed");
        return -1;
    }
    return sock_fd;
}
