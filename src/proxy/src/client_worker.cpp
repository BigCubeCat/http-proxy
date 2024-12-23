#include "client_worker.hpp"

#include <csignal>
#include <mutex>

#include <arpa/inet.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>

#include "const.hpp"
#include "forward.hpp"
#include "network.hpp"
#include "parser.hpp"
#include "status_check.hpp"


void *start_client_worker_routine(void *arg) {
    auto *worker = static_cast<client_worker *>(arg);
    if (worker == nullptr) {
        spdlog::critical("cannot start routine, arg is nullptr");
        throw std::runtime_error("start routine");
    }
    worker->start();
    return nullptr;
}

void client_worker::toggle_task(int fd) {
    m_lock.lock();
    m_fds.push(fd);
    m_lock.unlock();
    m_toggled_cond.notify_one();
}

void client_worker::start() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, nullptr);

    while (m_worker_is_running) {
        std::unique_lock<std::mutex> lock(m_toggle_lock);
        m_toggled_cond.wait(lock, [this]() { return !m_fds.empty(); });
        int current_client_fd = m_fds.front();
        m_fds.pop();
        lock.unlock();
        process_client_fd(current_client_fd);
    }
    spdlog::debug("worker finished");
}

void client_worker::stop() {
    m_worker_is_running = false;
    toggle_task(-1);
}

void client_worker::process_client_fd(int client_fd) {
    if (client_fd < 0) {
        return;
    }
    std::array<char, BUFFER_SIZE> buffer {};
    auto bytes_read = read(client_fd, buffer.data(), BUFFER_SIZE);
    if (bytes_read < 0) {
        warn_status(close(client_fd), "end connection");
        auto err_st = epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
        warn_status(err_st, "epoll_ctl DEL");
        return;
    }
    std::string method;
    std::string url;
    std::string host;
    if (!parse_request(buffer.data(), bytes_read, method, url, host)) {
        warn_status(close(client_fd), "close");
        return;
    }
    if (method == "GET") {
        auto response = process_get(client_fd, host, url, buffer.data());
        spdlog::debug("response size = {}", response.size());
        if (!response.empty()) {
            if (!send_all(client_fd, response)) {
                spdlog::error("cant send");
            }
        }
    }
    debug_status(close(client_fd), "close");
}


std::string client_worker::process_get(
    int client_fd,
    const std::string &host,
    const std::string &url,
    const std::string &request
) {
    spdlog::trace("GET method");

    auto cached_value = m_cache->get(url);
    if (cached_value != std::nullopt) {
        const auto &value = cached_value.value();
        // дополнительная проверка, чтобы перестраховаться
        if (value->is_finished()) {
            return value->data();
        }
    }
    spdlog::debug("not in cache");
    std::string ip_address;
    if (!resolve_host(host, ip_address)) {
        spdlog::error("failed to resolve host: {}", host);
        return "";
    }

    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(80);
    int sock_fd            = socket(AF_INET, SOCK_STREAM, 0);
    if (!error_status(sock_fd, "socket create failed")) {
        return "";
    }
    auto inet_st =
        inet_pton(AF_INET, ip_address.c_str(), &server_addr.sin_addr);
    if (!error_status(inet_st, "inet pton error")) {
        warn_status(close(sock_fd), "close");
        return "";
    }
    auto conn_st = connect(
        sock_fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)
    );
    warn_status(conn_st, "connection to server failed");
    if (conn_st < 0) {
        debug_status(close(sock_fd), "close socket failed");
        return "";
    }
    debug_status(
        send(sock_fd, request.c_str(), request.size(), 0), "send request"
    );

    std::ostringstream stream;
    recv_all(sock_fd, stream);
    debug_status(close(sock_fd), "close error");
    auto response    = stream.str();
    auto cache_value = std::make_shared<cached_item_t>();
    cache_value->lock_for_write();
    cache_value->push(response);
    cache_value->finish();
    cache_value->unlock_for_write();
    m_cache->set(url, cache_value);
    return response;
}

void client_worker::add_task(int fd) { }
