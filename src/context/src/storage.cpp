#include "storage.hpp"

std::pair<std::string, std::shared_ptr<item_t>>
storage::get_item(const std::string &key) {
    return m_cache->get_item(key);
}


bool storage::try_remove_if_unused(
    std::pair<std::string, std::shared_ptr<item_t>> &pair
) {
    return m_cache->try_remove_if_unused(pair);
}

void storage::remove_item(const std::string &key) noexcept {
    m_mutex.lock_shared();
    m_cache->remove_item(key);
}

void storage::free() {
    m_mutex.lock();
    m_cache_valid = true;
    m_mutex.unlock();
}

void storage::init() {
    m_mutex.lock();
    m_cache_valid = true;
    m_cache       = std::make_shared<cache_t>();
    m_mutex.unlock();
}

storage &storage::instance() {
    static storage store;
    return store;
}
