#include "thread_pool.hpp"

#include <cstddef>
#include <thread>

#include <spdlog/spdlog.h>

#include "pool/thread_pool_exception.hpp"

thread_pool_t::thread_pool_t(
    const std::vector<std::shared_ptr<worker_iface>> &tasks
)
    : m_count_pools(tasks.size()), m_tasks(tasks) {
    m_threads.resize(m_count_pools);
    spdlog::debug("build thread_pool_t with {} pools", m_count_pools);
}

void thread_pool_t::run(const std::function<void *(void *)> &start_routine) {
    for (size_t i = 0; i < m_count_pools; ++i) {
        spdlog::trace("starting thread {}", i);
        m_threads[i] = std::thread(
            // start_routine должна запускать соответствующий метод в tasks
            start_routine,
            m_tasks[i].get()
        );
        m_threads[i].detach();
    }
}

void thread_pool_t::add_task(int fd) {
    if (m_tasks[m_next_thread] == nullptr) {
        throw thread_pool_exception("worker is null");
    }
    m_tasks[m_next_thread]->add_task(fd);
    m_next_thread = (m_next_thread + 1) % m_count_pools;
}

// TODO: RENAME notify
void thread_pool_t::toggle_task(int fd) {
    auto thread_index_val = m_fd_map.find(fd);
    if (thread_index_val == m_fd_map.end()) {
        spdlog::error("unknown fd={}", fd);
        return;
    }
    auto thread_id = thread_index_val->second;
    m_tasks[thread_id]->toggle_task(fd);
}

thread_pool_t::~thread_pool_t() = default;
