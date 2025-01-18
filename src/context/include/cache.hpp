#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "item.hpp"

/*!
 * \brief Реализация кэша для хранения страниц
 * формат ключа: <host><url>
 */
class cache_t {
    std::unordered_map<std::string, std::shared_ptr<item_t>> m_hash_map;
    std::mutex m_mutex;

public:
    explicit cache_t() = default;

    std::pair<std::string, std::shared_ptr<item_t>>
    get_item(const std::string &key) noexcept;

    bool
    try_remove_if_unused(std::pair<std::string, std::shared_ptr<item_t>> &pair);

    void remove_item(const std::string &key) noexcept;
};
