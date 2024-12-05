#pragma once

#include <shared_mutex>
#include <string>

/*!
 * Значение, полученное из сети.
 * именно оно сохраняется в кэш
 */
class cached_item_t {
private:
    std::string m_request;
    std::string m_response;

    std::shared_mutex m_lock;

public:
    explicit cached_item_t();
};
