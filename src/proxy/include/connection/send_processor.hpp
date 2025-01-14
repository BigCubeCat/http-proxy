#pragma once

#include <string>

#include "connection/connection_processor.hpp"

/*!
 * \brief Объект для отсылания данных клиенту
 */
class send_processor_t : public connection_processor_iface {
private:
    int m_fd;
    bool m_end = false;
    std::string m_data;

    size_t m_total_bytes_sent = 0;
    size_t m_data_size;
    size_t m_bytes_left  = 0;
    ssize_t m_bytes_sent = 0;

public:
    ~send_processor_t() override = default;
    explicit send_processor_t(int fd, std::string data)
        : m_fd(fd), m_data(std::move(data)), m_data_size(data.size()) { }

    /*!
     * Возращает истину, если посылка данных закончена
     */
    bool process() override;

    std::string result() override {
        return "";
    }

    parser_stage stage() override {
        return parser_stage::WRITE_STAGE;
    }
};
