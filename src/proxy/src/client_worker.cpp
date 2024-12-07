#include "client_worker.hpp"

#include <mutex>

#include <spdlog/spdlog.h>
#include <sys/socket.h>

#include "const.hpp"
#include "parser.hpp"
#include "utils.hpp"

void client_worker::add_task(int fd) {
    m_lock.lock();
    m_fds.push(fd);
    m_lock.unlock();
}

void client_worker::toggle_task(int fd) {
    m_lock.lock();

    m_lock.unlock();
}

void client_worker::start() {
    while (true) {
        std::unique_lock<std::mutex> lock(m_toggle_lock);
        if (m_fds.empty()) {
            lock.unlock();
        }
        m_toggled_cond.wait(lock);
        int current_client_fd = m_fds.front();
        m_toggled_cond.notify_all();
        lock.unlock();
        process_client_fd(current_client_fd);
    }
}

void client_worker::process_client_fd(int client_fd) const {
    std::array<char, BUFFER_SIZE> buffer {};
    auto bytes_read = read(client_fd, buffer.data(), BUFFER_SIZE);
    if (bytes_read <= 0) {
        hs(close(client_fd), "end connection");
        hs(epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr),
           "epoll_ctl DEL");
    }
    else {
        std::string method;
        std::string url;
        std::string host;
        if (!parse_request(buffer.data(), bytes_read, method, url, host)) {
            hs(close(client_fd), "close");
            return;
        }

        if (method == "GET") {
            spdlog::trace("GET method");
            // тут надо сделать запрос к кэшу
            // auto it = cache.find(url);
            auto response = forward_request(host, buffer.data());
            if (!response.empty()) {
                // cache[url] = { .data = response, .timestamp = time(nullptr)
                // };
                hs(static_cast<int>(
                       send(client_fd, response.c_str(), response.size(), 0)
                   ),
                   "send in process_client_fd");
            }
        }
        hs(close(client_fd), "close");
    }
}
