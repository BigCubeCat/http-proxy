#pragma once


#include <chrono>
#include <cstddef>
#include <ctime>
#include <list>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include <spdlog/spdlog.h>

#include "timing.hpp"
#include "utils.hpp"

template <typename T>
using cache_map   = std::unordered_map<std::string, std::pair<T, timing>>;
using shared_lock = std::shared_lock<std::shared_mutex>;
using unique_lock = std::unique_lock<std::shared_mutex>;

// TODO: Time To Live

/*!
 * Реализация LRU-кэша со строковым ключом
 */
template <typename T>
class lru_cache_t {
private:
    long m_ttl;
    size_t m_size;
    cache_map<T> m_hash_map;
    std::list<std::string> m_usage_list;
    mutable std::shared_mutex m_lock;    // в отличе от std::mutex можно читать
                                         // сразу нескольким. mutable чтобы
                                         // можно было изменять даже из
                                         // константных методов

    void touch(cache_map<T>::iterator it) {
        m_usage_list.remove(it->first);
        it->second.second.used = current_unixtime();
        m_usage_list.emplace_front(it->first);
    }

    void delete_least_used_element() {
        auto key = m_usage_list.back();
        m_hash_map.erase(key);
        m_usage_list.pop_back();
    }

    bool is_expired(std::time_t creation_time) const {
        return static_cast<long>(current_unixtime() - creation_time) >= m_ttl;
    }

public:
    explicit lru_cache_t(size_t size, long ttl = 5)
        : m_size(size), m_ttl(ttl) { }

    /*!
     * \brief Получить элемент по ключу. В случае если элемента нет вернет
     * std::nullopt
     */
    std::optional<T> get(const std::string &key) {
        spdlog::debug("tring to get {}", key);
        shared_lock lock(m_lock);
        auto it = m_hash_map.find(key);
        if (it == m_hash_map.end()) {
            return std::nullopt;
        }
        spdlog::debug(
            "check expired {} {}", it->second.second.created, current_unixtime()
        );
        if (is_expired(it->second.second.created)) {
            return std::nullopt;
        }
        lock.unlock();
        unique_lock ulock(m_lock);
        touch(it);
        ulock.unlock();
        return it->second.first;
    }

    /*!
     * \brief Добавить элемент в кэш
     * В случае если элемент существет - обновит время использования
     * Иначе проверяет размер. Удаляет старые записи по необходимости и
     * добавляет новую запись.
     */
    void set(const std::string &key, T element) {
        spdlog::debug("tring to set {}", key);
        unique_lock lock(m_lock);
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
        m_hash_map[key] = {
            element, { current_unixtime(), current_unixtime() }
        };
        lock.unlock();
    }
};
