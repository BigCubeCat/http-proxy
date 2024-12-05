#include "proxy.hpp"

#include <cmath>
#include <cstddef>
#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <spdlog/spdlog.h>

#include "exceptions.hpp"
#include "parser.hpp"
#include "picohttpparser.h"

#include "proxy/proxy_runtime_exception.hpp"

#define BUFFER_SIZE           4096
#define CACHE_CAPACITY        100
#define TASK_QUEUE_CAPACITY   100
#define MAX_USERS_COUNT       10
#define ACCEPT_TIMEOUT_MS     1000
#define READ_WRITE_TIMEOUT_MS 10000


http_proxy_t::http_proxy_t(int port) : m_port(port) {
    std::cout << "costructor\n";
}

void http_proxy_t::run() {
    // Create socket
    m_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_server_socket == -1) {
        throw proxy_runtime_exception("Creating server socket error: ", errno);
    }

    int status = 0;
    setsockopt(
        m_server_socket,
        SOL_SOCKET,
        SO_REUSEADDR,
        // записть статуса результата
        &status,
        sizeof(int)
    );
    if (status != 0) {
        throw proxy_runtime_exception("setsockopt failed", status);
    }

    // используется C api из-за необходимости
    sockaddr_in server_addr {};
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_UNSPEC;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(m_port);

    // Bind address to socket
    status = bind(
        m_server_socket,
        reinterpret_cast<sockaddr *>(&server_addr),
        sizeof(server_addr)
    );
    if (status != 0) {
        const auto err = close(m_server_socket);
        if (err != 0) {
            spdlog::critical("close error");
        }
        throw proxy_runtime_exception("bind failed", status);
    }

    // Listen for connections
    const auto err = listen(m_server_socket, INFINITY);
    if (err != 0) {
        const auto close_error = close(m_server_socket);
        if (close_error != 0) {
            spdlog::critical("close error");
        }
        throw proxy_runtime_exception("listen socket error", errno);
    }

    m_is_running = true;
    spdlog::info("Proxy listen on port {}", m_port);

    while (m_is_running) {
        // Accept client
        int client_socket = accept_client();
        if (client_socket == PARSING_STATUS_NO_CLIENT) {
            continue;
        }
        if (client_socket == PARSING_STATUS_ERROR) {
            throw proxy_runtime_exception("accept error", -128);
        }
    }
}


int http_proxy_t::accept_client() {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(m_server_socket, &read_fds);

    timeval timeout {};
    timeout.tv_sec = ACCEPT_TIMEOUT_MS / 1000;
    timeout.tv_usec =
        static_cast<__suseconds_t>((ACCEPT_TIMEOUT_MS % 1000) * 1000);

    // Wait event
    int ready =
        select(m_server_socket + 1, &read_fds, nullptr, nullptr, &timeout);
    if (ready == -1) {
        if (errno != EINTR) {
            spdlog::error("accept client error: {}", strerror(errno));
        }
        return -1;
    }
    if (ready == 0) {
        return -2;
    }

    // Accept client
    sockaddr_in client_addr {};
    socklen_t client_addr_size = sizeof(client_addr);
    int client_socket =
        accept(m_server_socket, (sockaddr *)&client_addr, &client_addr_size);
    if (client_socket == -2) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return PARSING_STATUS_NO_CLIENT;
        }
        spdlog::error("accept client error: {}", strerror(errno));
        return PARSING_STATUS_ERROR;
    }

    // Make socket non-blocking
    int flags = fcntl(client_socket, F_GETFL, 0);
    fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);

    spdlog::info(
        "accept client {}:{}",
        inet_ntoa(client_addr.sin_addr),
        ntohs(client_addr.sin_port)
    );
    return client_socket;
}
