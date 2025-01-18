#include "thread_pool.hpp"

#include <csignal>
#include <cstddef>
#include <thread>

#include <spdlog/spdlog.h>

#include "pool/thread_pool_exception.hpp"

thread_pool_t::thread_pool_t(int n) : m_count_pools(n) {
    m_threads.resize(m_count_pools);
    spdlog::debug("build thread_pool_t with {} pools", m_count_pools);
}

void thread_pool_t::run(
    const std::function<void *(void *)> &start_routine, bool skip_first
) {
    spdlog::debug(
        "cout_pools={}; tasks_size={}", m_count_pools, m_tasks.size()
    );
    int begin = (skip_first) ? 1 : 0;
    for (size_t i = begin; i < m_count_pools; ++i) {
        spdlog::trace("starting thread {}", i);
        // start_routine должна запускать соответствующий метод в tasks
        m_threads[i] = std::thread(start_routine, m_tasks[i].get());
        m_threads[i].detach();
    }
}
void thread_pool_t::stop() {
    for (const auto &task : m_tasks) {
        task->stop();
    }
}

void thread_pool_t::add_task(int fd) {
    if (m_tasks[m_next_thread] == nullptr) {
        throw thread_pool_exception("worker is null");
    }
    m_fd_map[fd] = m_next_thread;
    m_tasks[m_next_thread]->add_task(fd);
    m_next_thread = (m_next_thread + 1) % m_count_pools;
}

void thread_pool_t::notify(int fd) {
    auto thread_index_val = m_fd_map.find(fd);
    if (thread_index_val == m_fd_map.end()) {
        spdlog::critical("unknown fd={}", fd);
        return;
    }
    auto thread_id = thread_index_val->second;
    m_tasks[thread_id]->toggle_task(fd);
}


void thread_pool_t::set_tasks(std::vector<std::shared_ptr<worker_iface>> tasks
) {
    m_tasks = std::move(tasks);
}

thread_pool_t::~thread_pool_t() = default;
