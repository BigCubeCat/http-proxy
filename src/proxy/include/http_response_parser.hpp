#pragma once

#include <array>
#include <strstream>

#include "cache.hpp"
#include "const.hpp"

#include "connection/connection_processor.hpp"


/*!
 * \brief объект, обрабатывающий подключение
 */
class http_response_processor_t {
private:
    parser_stage m_stage = parser_stage::READ_STAGE;
    int m_client_fd;
    int m_status = 400;

    std::array<char, BUFFER_SIZE> m_buffer {};
    std::string m_data;

    std::string m_request;

    std::string m_host;
    std::string m_url;
    std::string m_method;

    std::ostrstream m_stream;

    std::string m_result;

    cache_t *m_cache_ptr;

    /*!
     * \brief Функция для отправки строки по сети
     */
    bool send_all();

    /*!
     * \brief Функция для получения данных из сети
     */
    void recv_all(int fd);

public:
    explicit http_response_processor_t(int fd, cache_t *cache_ptr);

    bool process();

    void switch_stage(const parser_stage &stage);

    [[nodiscard]] parser_stage stage() const;

    /*!
     * Получение response
     */
    bool receive();

    bool parse_client_request();

    void set_cached_data(std::string data);

    [[nodiscard]] std::string data() const;

    [[nodiscard]] std::string url() const;
};
