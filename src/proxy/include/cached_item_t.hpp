#pragma once

#include <shared_mutex>
#include <string>

/*!
 * Значение, полученное из сети.
 * именно оно сохраняется в кэш
 */
class cached_item_t {
private:
    std::shared_mutex m_mutex;

    bool m_valid    = true;
    bool m_finished = false;
    /*! \brief Результат */
    std::string m_data;

public:
    explicit cached_item_t() = default;

    [[nodiscard]] bool is_finished();

    std::string data();

    void push(const std::string &appendix);

    void finish();

    void lock_for_write();

    void unlock_for_write();
};
