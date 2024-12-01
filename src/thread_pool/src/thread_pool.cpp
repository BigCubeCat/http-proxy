#include "thread_pool.hpp"

#include <cstddef>
#include <thread>

#include <spdlog/spdlog.h>

#include "exceptions.hpp"

#include "pool/init.hpp"

thread_pool_t::thread_pool_t(
    const std::vector<std::shared_ptr<worker_iface>> &tasks
)
    : m_tasks(tasks), m_count_pools(tasks.size()) {
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

void thread_pool_t::add_task(void *arg) {
    if (m_tasks[m_next_thread] == nullptr) {
        throw thread_pool_exception("worker is null");
    }
    m_tasks[m_next_thread]->add_task(arg);
    m_next_thread = (m_next_thread + 1) % m_count_pools;
}
