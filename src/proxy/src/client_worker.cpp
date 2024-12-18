#include "client_worker.hpp"

#include <mutex>

#include <spdlog/spdlog.h>
#include <sys/socket.h>

#include "const.hpp"
#include "parser.hpp"
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
    while (true) {
        std::unique_lock<std::mutex> lock(m_toggle_lock);
        m_toggled_cond.wait(lock, [this]() { return !m_fds.empty(); });
        int current_client_fd = m_fds.front();
        m_fds.pop();
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
                /// ТУТ ХУЙНЯ
                auto sent_now =
                    send(client_fd, response.c_str(), response.size(), 0);
                auto sent_values   = sent_now;
                auto response_size = response.size();
                while (sent_values != response_size) {
                    spdlog::trace(
                        "response_size = {}; sent_values = {}",
                        response_size,
                        sent_values
                    );
                    response = response.substr(sent_now, response.size());
                    spdlog::trace("trace cuttetA");
                    sent_now = send(
                        // отпарвка данных в клиентский дескриптор частями
                        client_fd,
                        response.c_str(),
                        response.size(),
                        0
                    );
                    sent_values += sent_now;
                }
            }
        }
        hs(close(client_fd), "close");
    }
}

void client_worker::add_task(int fd) { }
