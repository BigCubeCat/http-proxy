#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>

#include <unistd.h>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <gtest/gtest.h>

#include "task.hpp"
#include "thread_pool.hpp"


const int total_tasks = 5;
/*!
 * Имплиментация интерфейса для unit-тестирования
 */
class test_worker : public worker_iface {
public:
    int m_total_tasks = total_tasks;
    std::vector<size_t> m_res;
    int m_current = 0;
    int m_num;
    int m_value = -1;
    std::mutex m_mutex;
    static size_t factorial(size_t value) {
        size_t res = 1;
        for (size_t i = 2; i <= value; ++i) {
            res *= i;
        }
        return res;
    }
    explicit test_worker(int num) : m_num(num) {
        m_res.resize(m_num);
    }
    virtual ~test_worker() {
        spdlog::trace("task {} destructor", m_num);
    }
    static void *start_routine(void *arg) {
        auto *self = static_cast<test_worker *>(arg);
        if (!self) {
            throw std::runtime_error("self is null;");
        }
        self->start();
        return nullptr;
    }
    void start() override {
        spdlog::debug("start()");
        while (m_current < m_total_tasks) {
            if (m_value == -1) {
                usleep(100);
                continue;
            }
            m_mutex.lock();
            m_res[m_current] = factorial(m_value);
            spdlog::debug("factorial={}", m_res[m_current]);
            m_current++;
            m_value = -1;
            m_mutex.unlock();
        }
    }
    void add_task(void *arg) override {
        while (m_value != -1) {
            usleep(100);
        }
        m_mutex.lock();
        m_value = *static_cast<int *>(arg);
        m_mutex.unlock();
    }
};

TEST(ThreadPool, Basic) {
    spdlog::set_level(spdlog::level::err);
    int n = 2;
    std::vector<std::shared_ptr<worker_iface>> workers;
    workers.reserve(n);
    for (int i = 0; i < n; ++i) {
        workers.push_back(std::make_shared<test_worker>(5));
    }
    spdlog::warn("{}", workers[0].get() == nullptr);
    auto pool = thread_pool_t(workers);
    spdlog::warn("{}", workers[0].get() == nullptr);
    pool.run(test_worker::start_routine);
    std::vector<std::shared_ptr<int>> args(static_cast<size_t>(total_tasks * n)
    );
    int k = 0;
    for (int i = 2; i < total_tasks + 2; ++i) {
        spdlog::debug("add_task({})", i);
        args[k]     = std::make_shared<int>(i);
        args[k + 1] = std::make_shared<int>(i);
        pool.add_task(args[k].get());
        pool.add_task(args[k].get());
        k += 2;
    }
    spdlog::trace("waiting");
    usleep(1000);
    auto res1 = dynamic_cast<test_worker *>(workers[0].get())->m_res;
    auto res2 = dynamic_cast<test_worker *>(workers[0].get())->m_res;
    spdlog::trace("got results");
    EXPECT_EQ(res1, res2);
    std::vector<size_t> true_res = { 2, 6, 24, 120, 720 };
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(res1[i], true_res[i]);
    }
}
