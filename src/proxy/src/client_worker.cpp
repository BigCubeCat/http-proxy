#include "client_worker.hpp"

#include <csignal>

#include <arpa/inet.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>

#include "network.hpp"
#include "status_check.hpp"
#include "utils.hpp"


void *start_client_worker_routine(void *arg) {
    auto *worker = static_cast<client_worker *>(arg);
    if (worker == nullptr) {
        spdlog::critical("cannot start routine, arg is nullptr");
        throw std::runtime_error("start routine");
    }
    disable_signals();
    worker->start();
    return nullptr;
}

void client_worker::start() {
    m_proxy_inst.start_server_loop();
    spdlog::debug("worker finished");
}

void client_worker::stop() {
    m_worker_is_running = false;
}

void client_worker::add_task(int fd) {
    m_proxy_inst.add_server_socket(fd);
}

int client_worker::accept_client() {
    auto client_fd = accept(m_listen_fd, nullptr, nullptr);
    if (!error_status(client_fd, "accept failed")) {
        return -1;
    }
    set_not_blocking(client_fd);
    m_proxy_inst.add_client_socket(client_fd);
    return client_fd;
}
