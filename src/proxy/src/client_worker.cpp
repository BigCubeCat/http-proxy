#include "client_worker.hpp"

#include <csignal>

#include <arpa/inet.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>

#include "http_response_parser.hpp"
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
    auto terminating_lambda = [](int epoll_fd, int client_fd) {
        warn_status(close(client_fd), "end connection");
        auto err_st = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
        warn_status(err_st, "epoll_ctl DEL");
    };
    if (client_fd < 0) {
        terminating_lambda(m_epoll_fd, client_fd);
        return;
    }
    auto client_processor = http_response_processor_t(client_fd);
    if (!client_processor.parse_client_request()) {
        terminating_lambda(m_epoll_fd, client_fd);
        return;
    }
    auto cached_value = m_cache->get(client_processor.url());
    if (cached_value != std::nullopt) {
        client_processor.set_cached_data(cached_value.value());
        client_processor.process();
        terminating_lambda(m_epoll_fd, client_fd);
        return;
    }
    auto status = client_processor.receive();
    if (!status) {
        terminating_lambda(m_epoll_fd, client_fd);
        return;
    }
    m_cache->set(client_processor.url(), client_processor.data());
    if (!client_processor.process()) {
        spdlog::error("");
    }
}

void client_worker::add_task(int fd) { }
