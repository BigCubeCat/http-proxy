#pragma once

#include <shared_mutex>
#include <string>

/*!
 * Значение, полученное из сети.
 * именно оно сохраняется в кэш
 */
class cached_item_t {
private:
    bool m_valid  = true;
    bool m_full   = false;
    size_t m_size = 0;

    /* \brief Результат */
    std::string m_data;

    std::shared_mutex m_lock;

public:
    explicit cached_item_t() = default;

    /*!
     * возращает статуса валидности
     */
    [[nodiscard]] bool valid();

    /*!
     * возращает статуса выполненности загрузки
     */
    [[nodiscard]] bool full();

    /*!
     * возращает размер значения
     */
    [[nodiscard]] size_t size();

    /*!
     * \brief Возращает значение элемента
     */
    [[nodiscard]] std::string data();

    /*!
     * \bried Статус мьютекса
     */
    bool is_busy();

    /*!
     * \brief Записывает данные в конец значениея
     * \param[in] buffer данные, которые будут добавленны в конец
     * \param[in] is_end true, если записаны все данные
     */
    void push(const std::string &buffer, bool is_end);

    /*!
     * \brief ставит флаг invalid
     */
    void invalidate();
};
