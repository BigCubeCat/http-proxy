#include "cache.hpp"

#include "utils.hpp"

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

void cache_t::try_clean() {
    if (m_hash_map.size() < m_max_size) {
        return;
    }
    m_mutex.lock();
    auto ct = current_time();
    for (auto it = m_hash_map.begin(); it != m_hash_map.end();) {
        if (it->second->unixtime() > 0 && it->second->get_pin_count() == 0
            && ct - it->second->unixtime() > m_ttl) {
            it = m_hash_map.erase(it);
        }
        else {
            ++it;
        }
    }
    m_mutex.unlock();
}

void cache_t::update_params(long ttl, size_t size) {
    m_ttl      = ttl;
    m_max_size = size;
}
