#include "cached_item_t.hpp"

bool cached_item_t::is_finished() {
    m_mutex.lock_shared();
    auto res = m_finished;
    m_mutex.unlock_shared();
    return res;
}

std::string cached_item_t::data() {
    m_mutex.lock_shared();
    if (m_finished) {
        return m_data;
    }
    m_mutex.unlock_shared();
    return "";
}

void cached_item_t::push(const std::string &appendix) {
    m_data += appendix;
}

void cached_item_t::finish() {
    m_finished = true;
}

void cached_item_t::lock_for_write() {
    m_mutex.lock();
}

void cached_item_t::unlock_for_write() {
    m_mutex.unlock();
}
