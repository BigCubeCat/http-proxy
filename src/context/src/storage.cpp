#include "storage.hpp"

#include <spdlog/spdlog.h>

std::pair<std::string, std::shared_ptr<item_t>>
storage::get_item(const std::string &key) {
    return m_cache->get_item(key);
}

void storage::remove_item(const std::string &key) noexcept {
    m_cache->remove_item(key);
}

void storage::free() {
    m_cache_valid = true;
}

void storage::init(long ttl, size_t size) {
    m_cache_valid = true;
    m_cache       = std::make_shared<cache_t>();
    m_cache->update_params(ttl, size);
}

storage &storage::instance() {
    static storage store;
    return store;
}
