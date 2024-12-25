#pragma once


#include <array>

#include "const.hpp"

/*!
 * \brief объект, обрабатывающий подключение
 */
class http_response_processor_t {
private:
    int m_client_fd;
    int m_status = 400;

    std::array<char, BUFFER_SIZE> m_buffer {};
    std::string m_data;

    std::string m_request;

    std::string m_host;
    std::string m_url;
    std::string m_method;

    /*!
     * \brief Функция для отправки строки по сети
     */
    bool send_all();

    /*!
     * \brief Функция для получения данных из сети
     */
    void recv_all(int fd);

public:
    explicit http_response_processor_t(int fd);

    bool process();

    /*!
     * Получение response
     */
    bool receive();

    bool parse_client_request();

    void set_cached_data(std::string data);

    [[nodiscard]] std::string data() const;

    [[nodiscard]] std::string url() const;
};
