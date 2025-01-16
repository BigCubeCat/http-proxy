#include "client_worker.hpp"

#include <csignal>

#include <arpa/inet.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>

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
    spdlog::debug("adding task {}", fd);
    m_proxy_inst.add_client_socket(fd);
}
