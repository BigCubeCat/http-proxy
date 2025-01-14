#include "client_worker.hpp"

#include <csignal>

#include <arpa/inet.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>

#include "http_response_parser.hpp"
#include "network.hpp"
#include "status_check.hpp"
#include "utils.hpp"

#include "connection/connection_processor.hpp"


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
    run();
    spdlog::debug("worker finished");
}

void client_worker::stop() {
    m_worker_is_running = false;
}

void client_worker::run() {
    int nfds;
    while (m_worker_is_running) {
        nfds = m_epoll.select();
        for (int i = 0; i < nfds; ++i) {
            if (m_is_root && m_epoll[i].data.fd == m_listen_fd) {
                // Принимаем новое подклчение, если мы root и если произошло
                // событие на m_listen_fd
                auto client_fd = accept_client();
                if (!error_status(client_fd, "failed to accept client")) {
                    continue;
                }
                // отдаем задачу на перераспределение
                m_pool->add_task(client_fd);
                continue;
            }
            process_client_fd(m_epoll[i].data.fd, m_epoll[i].events);
        }
    }
}

void client_worker::add_task(int fd) {
    m_epoll.register_fd(fd, EPOLLIN);
}

void client_worker::process_client_fd(int client_fd, uint32_t events) {
    auto terminating_lambda = [this](int epoll_fd, int client_fd) {
        m_processor_map.erase(client_fd);
        debug_status(close(client_fd), "end connection");
        debug_status(
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr),
            "epoll_ctl delete"
        );
    };
    if (client_fd < 0) {
        terminating_lambda(m_epoll.epoll_fd(), client_fd);
        return;
    }
    auto processor_value = m_processor_map.find(client_fd);
    if (processor_value == m_processor_map.end()) {
        // Создаем процессор
        auto client_processor =
            std::make_shared<http_response_processor_t>(client_fd);
        m_processor_map[client_fd] = client_processor;
        return;
    }
    auto client_processor = processor_value->second;
    if (client_processor->stage() == parser_stage::BEGIN_STAGE) {
        // Значит, что работа еще не началсь, кэш не проверялся
        auto cached_value = m_cache->get(client_processor->url());
        if (cached_value != std::nullopt) {
            client_processor->set_cached_data(cached_value.value());
            client_processor->switch_stage(parser_stage::WRITE_STAGE
            );    // ставим в запись
            return;
        }
        client_processor->switch_stage(parser_stage::READ_STAGE);
    }
    else if (client_processor->stage() == parser_stage::END_STAGE) {
        m_cache->set(client_processor->url(), client_processor->data());
        terminating_lambda(m_epoll.epoll_fd(), client_fd);
        return;
    }
    client_processor->process();
}

int client_worker::accept_client() {
    auto client_fd = accept(m_listen_fd, nullptr, nullptr);
    if (!error_status(client_fd, "accept failed")) {
        return -1;
    }
    set_not_blocking(client_fd);
    register_fd(m_epoll.epoll_fd(), client_fd);
    return client_fd;
}
