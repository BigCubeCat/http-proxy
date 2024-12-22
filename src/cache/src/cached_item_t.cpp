#include "cached_item_t.hpp"

#include <spdlog/spdlog.h>


bool cached_item_t::valid() {
    m_lock.lock_shared();
    auto status = m_valid;
    m_lock.unlock_shared();
    return status;
}

bool cached_item_t::full() {
    m_lock.lock_shared();
    auto status = m_full && m_valid;
    m_lock.unlock_shared();
    return status;
}

size_t cached_item_t::size() {
    m_lock.lock_shared();
    auto size = m_size;
    m_lock.unlock_shared();
    return size;
}

std::string cached_item_t::data() {
    m_lock.lock_shared();
    auto result = m_data;
    spdlog::trace("cache data loaded");
    m_lock.unlock_shared();
    return result;
}

bool cached_item_t::is_busy() {
    return !m_lock.try_lock();
}

void cached_item_t::push(const std::string &buffer, bool is_end) {
    m_lock.lock();
    m_data += buffer;
    m_size = m_data.size();
    m_full = is_end;
    if (is_end) {
        m_valid = true;
    }
    m_lock.unlock();
}

/*!
 * \brief ставит флаг invalid
 */
void cached_item_t::invalidate() {
    m_lock.lock();
    m_valid = true;
    m_lock.unlock();
}
