#include "network.hpp"

#include <fcntl.h>

#include <arpa/inet.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "parser.hpp"
#include "status_check.hpp"

bool set_not_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, SO_REUSEPORT);
    if (flags == -1) {
        spdlog::critical("fcntl F_GETFL");
        return false;
    }
    flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (flags == -1) {
        spdlog::critical("fcntl F_SETFL");
        return false;
    }
    return true;
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

int open_http_socket(const std::string &host, int &res) {
    std::string ip_address;
    int port;
    if (!resolve_host(host, ip_address, port)) {
        spdlog::error("failed to resolve host: {}", host);
        return -1;
    }

    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);
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
    res = connect(
        sock_fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)
    );
    warn_status(res, "connection to server failed");
    return sock_fd;
}
