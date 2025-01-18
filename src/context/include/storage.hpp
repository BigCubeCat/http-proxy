#pragma once

#include <memory>

#include "cache.hpp"

/*!
 * \brief Обертка для cache_t, является singleton
 */
class storage : public cache_t {
private:
    storage()  = default;
    ~storage() = default;

    bool m_cache_valid = false;

    std::shared_ptr<cache_t> m_cache;

public:
    storage(const storage &)            = delete;
    storage &operator=(const storage &) = delete;

    std::pair<std::string, std::shared_ptr<item_t>>
    get_item(const std::string &key);

    void remove_item(const std::string &key) noexcept;

    void free();

    void init(long ttl, size_t size);

    static storage &instance();
};
