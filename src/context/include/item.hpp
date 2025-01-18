#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "wait_context.hpp"


/*!
 * \brief значение кэша
 */
class item_t {
    std::string m_data;
    bool m_started;
    bool m_completed;
    std::mutex m_lock;
    int m_pin_count;
    std::vector<wait_context_t> m_waiting_clients;

public:
    explicit item_t();

    /*!
     * \brief увеличить число подключенных клиентов на 1
     */
    void pin();

    /*!
     * removes fd from waiting_clients if fd > -1;
     */
    void unpin(int fd);

    [[nodiscard]] int get_pin_count() const noexcept {
        return m_pin_count;
    }

    void put_data(const std::string &s) noexcept;

    /*!
    return from 0 up to limit if download still in progres
    return -1 if offset == data.lenght && complited
    */
    int get_data(
        std::string &dst,
        size_t offset,
        size_t limit,
        const wait_context_t &wait_context
    ) noexcept;

    void set_completed(bool val) noexcept;

    [[nodiscard]] bool is_compleated() const noexcept {
        return m_completed;
    }

    bool set_started(bool val) noexcept;

    [[nodiscard]] bool is_started() const noexcept {
        return m_started;
    }
};
