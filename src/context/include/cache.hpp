#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "const.hpp"
#include "item.hpp"

/*!
 * \brief Реализация кэша для хранения страниц
 * формат ключа: <host><url>
 */
class cache_t {
    std::unordered_map<std::string, std::shared_ptr<item_t>> m_hash_map;
    std::mutex m_mutex;

    long m_ttl        = CACHE_TTL;
    size_t m_max_size = 100;

public:
    explicit cache_t() = default;

    std::pair<std::string, std::shared_ptr<item_t>>
    get_item(const std::string &key) noexcept;

    void remove_item(const std::string &key) noexcept;

    /*!
     * \brief проходимся по кэшу, удаляем мусор
     */
    void try_clean();

    void update_params(long ttl, size_t size);
};
