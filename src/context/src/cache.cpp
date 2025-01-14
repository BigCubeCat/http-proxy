#include "cache.hpp"

std::pair<std::string, std::shared_ptr<item_t>>
cache_t::get_item(const std::string &key) noexcept {
    m_mutex.lock();
    auto it = m_hash_map.find(key);
    if (it == m_hash_map.end()) {
        std::shared_ptr<item_t> item = std::make_shared<item_t>();
        auto result                  = m_hash_map.emplace(key, item);
        result.first->second->pin();
        m_mutex.unlock();
        return *result.first;
    }
    it->second->pin();
    m_mutex.unlock();
    return *it;
}

void cache_t::remove_item(const std::string &key) noexcept {
    m_mutex.lock();
    m_hash_map.erase(key);
    m_mutex.unlock();
}

bool cache_t::try_remove_if_unused(
    std::pair<std::string, std::shared_ptr<item_t>> &pair
) {
    m_mutex.lock();
    if (pair.second->get_pin_count() == 0) {
        m_hash_map.erase(pair.first);
        m_mutex.unlock();
        return true;
    }
    m_mutex.unlock();
    return false;
}
