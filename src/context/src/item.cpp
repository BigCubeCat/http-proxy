#include "item.hpp"

item_t::item_t() : m_started(false), m_completed(false), m_pin_count(0) { }

void item_t::pin() {
    m_lock.lock();
    ++m_pin_count;
    m_lock.unlock();
}

void item_t::unpin(int fd) {
    m_lock.lock();
    if (m_pin_count > 0) {
        --m_pin_count;
    }
    if (fd <= -1) {
        m_lock.unlock();
        return;
    }
    for (auto it = m_waiting_clients.begin(); it < m_waiting_clients.end();
         ++it) {
        if (it->fd() == fd) {
            m_waiting_clients.erase(it);
            break;
        }
    }
    m_lock.unlock();
}

void item_t::put_data(const std::string &s) noexcept {
    m_lock.lock();
    m_data.append(s);
    size_t len = m_waiting_clients.size();
    for (size_t i = 0; i < len; ++i) {
        m_waiting_clients[i].notify();
    }
    m_waiting_clients.clear();
    m_lock.unlock();
}

int item_t::get_data(
    std::string &dst,
    size_t offset,
    size_t limit,
    const wait_context_t &wait_context
) noexcept {
    m_lock.lock();
    if (m_data.length() == offset && m_completed) {
        m_lock.unlock();
        return -1;
    }

    if (m_data.length() <= offset) {
        m_waiting_clients.push_back(wait_context);
        wait_context.get_context_ptr()->change_descriptor_mode(
            wait_context.fd(), 0
        );
        m_lock.unlock();
        return 0;
    }
    dst     = m_data.substr(offset, limit);
    int ret = static_cast<int>(dst.length());
    m_lock.unlock();
    return ret;
}

void item_t::set_completed(bool val) noexcept {
    m_lock.lock();
    m_completed = val;
    if (m_completed) {
        size_t len = m_waiting_clients.size();
        for (size_t i = 0; i < len; ++i) {
            m_waiting_clients[i].notify();
        }
        m_waiting_clients.clear();
    }
    m_lock.unlock();
}

bool item_t::set_started(bool val) noexcept {
    m_lock.lock();
    bool prev = m_started;
    m_started = val;
    m_lock.unlock();
    return prev;
}
