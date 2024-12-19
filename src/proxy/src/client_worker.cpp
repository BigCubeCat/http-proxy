#include "client_worker.hpp"

#include <csignal>
#include <mutex>

#include <spdlog/spdlog.h>
#include <sys/socket.h>

#include "const.hpp"
#include "network.hpp"
#include "parser.hpp"
#include "status_check.hpp"
#include "utils.hpp"


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

void client_worker::process_client_fd(int client_fd) const {
    if (client_fd < 0) {
        return;
    }
    std::array<char, BUFFER_SIZE> buffer {};
    auto bytes_read = read(client_fd, buffer.data(), BUFFER_SIZE);
    if (bytes_read <= 0) {
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
        spdlog::trace("GET method");
        std::string response;
        auto value = m_cache->get(url);
        if (value == std::nullopt) {
            response = forward_request(host, buffer.data());
            m_cache->set(url, response);
        }
        else {
            response = value.value();
        }
        if (!response.empty()) {
            if (!send_all(client_fd, response)) {
                spdlog::error("cant send");
            }
        }
    }
    debug_status(close(client_fd), "close");
}

void client_worker::add_task(int fd) { }
