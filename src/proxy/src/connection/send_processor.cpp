#include "connection/send_processor.hpp"

#include <spdlog/spdlog.h>
#include <sys/socket.h>

#include "const.hpp"

bool send_processor_t::process() {
    if (m_total_bytes_sent < m_data_size) {
        m_bytes_left = m_data_size - m_total_bytes_sent;
        m_bytes_sent = send(
            m_fd,
            m_data.data() + m_total_bytes_sent,
            BUFFER_SIZE > m_bytes_left ? m_bytes_left : BUFFER_SIZE,
            0
        );
        if (m_bytes_sent < 0) {
            spdlog::error("error sending data: {} {}", strerror(errno), errno);
            return true;
        }
        m_total_bytes_sent += static_cast<size_t>(m_bytes_sent);
    }
    return !(m_total_bytes_sent < m_data_size);
}
