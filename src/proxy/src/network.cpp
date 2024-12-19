#include "network.hpp"

#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "const.hpp"

bool send_all(int socket, const std::string &data) {
    size_t total_bytes_sent = 0;
    size_t data_size        = data.size();

    while (total_bytes_sent < data_size) {
        size_t bytes_left  = data_size - total_bytes_sent;
        ssize_t bytes_sent = send(
            socket, data.data() + total_bytes_sent, bytes_left, MSG_DONTWAIT
        );
        if (bytes_sent <= 0) {
            spdlog::error("error sending data: {}", strerror(errno));
            return false;
        }
        total_bytes_sent += static_cast<size_t>(bytes_sent);
    }
    return true;
}


void recv_all(int socket, std::ostringstream &response) {
    std::array<char, BUFFER_SIZE> buffer {};
    ssize_t bytes_read = recv(socket, buffer.data(), BUFFER_SIZE, 0);
    spdlog::trace("bytes_read = {}", bytes_read);
    while (bytes_read > 0) {
        response.write(buffer.data(), bytes_read);
        bytes_read = recv(socket, buffer.data(), BUFFER_SIZE, 0);
    }
}
