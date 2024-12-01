#pragma once


#include <cstddef>
#include <ctime>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include <spdlog/spdlog.h>

#include "utils.hpp"


/*!
 * Реализация LRU-кэша со строковым ключом
 */
template <typename T>
class lru_cache_t {
private:
    size_t m_size;
    std::unordered_map<std::string, std::pair<T, std::time_t>> m_hash_map;
    std::list<std::string> m_usage_list;

    void touch(
        std::unordered_map<std::string, std::pair<T, std::time_t>>::iterator it
    ) {
        m_usage_list.remove(it->first);
        it->second.second = current_unixtime();
        m_usage_list.emplace_front(it->first);
    }

    void delete_least_used_element() {
        auto key = m_usage_list.back();
        m_hash_map.erase(key);
        m_usage_list.pop_back();
    }

public:
    explicit lru_cache_t(size_t size) : m_size(size) { }

    /*!
     * \brief Получить элемент по ключу. В случае если элемента нет вернет
     * std::nullopt
     */
    std::optional<T> get(const std::string &key) {
        auto it = m_hash_map.find(key);
        if (it == m_hash_map.end()) {
            return std::nullopt;
        }
        touch(it);
        return it->second.first;
    }

    /*!
     * \brief Добавить элемент в кэш
     * В случае если элемент существет - обновит время использования
     * Иначе проверяет размер. Удаляет старые записи по необходимости и
     * добавляет новую запись.
     */
    void set(const std::string &key, T element) {
        auto it = m_hash_map.find(key);
        if (it != m_hash_map.end()) {
            // если сущетвует - обновить использование
            touch(it);
        }
        else {
            spdlog::debug("adding new element {}={}", key, element);
            while (m_usage_list.size() >= m_size) {
                delete_least_used_element();
            }
            m_usage_list.emplace_front(key);
        }
        m_hash_map[key] = { element, current_unixtime() };
        spdlog::debug("{} {}", m_hash_map[key].first, m_hash_map[key].second);
    }
};
