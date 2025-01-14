#pragma once

#include <ctime>

/*!
 * Вспомогательная структура для учета времени жизни записи
 */
struct timing {
    /*!
     * Время, когда запись была использована
     */
    std::time_t used;
    /*!
     * Время создания записи
     */
    std::time_t created;
};
