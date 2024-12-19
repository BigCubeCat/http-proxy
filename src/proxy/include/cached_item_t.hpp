#pragma once

#include <shared_mutex>
#include <string>

/*!
 * Значение, полученное из сети.
 * именно оно сохраняется в кэш
 */
class cached_item_t {
private:
    bool m_finished = false;
    /* \brief url */
    std::string m_url;
    /* \brief Результат */
    std::string m_response;

    std::shared_mutex m_lock;

public:
    explicit cached_item_t();
};
