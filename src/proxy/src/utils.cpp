#include "utils.hpp"

#include <cstring>
#include <sstream>
#include <string>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "const.hpp"
#include "network.hpp"
#include "parser.hpp"
#include "status_check.hpp"

/*!
 * \brief Отправка запроса на целевой сервер
 */
std::string
forward_request(const std::string &host, const std::string &request) {
    std::string ip_address;
    if (!resolve_host(host, ip_address)) {
        spdlog::error("failed to resolve host: {}", host);
        return "";
    }

    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(80);
    int sock               = socket(AF_INET, SOCK_STREAM, 0);
    if (!error_status(sock, "socket create failed")) {
        return "";
    }
    auto inet_st =
        inet_pton(AF_INET, ip_address.c_str(), &server_addr.sin_addr);
    if (!error_status(inet_st, "inet pton error")) {
        warn_status(close(sock), "close");
        return "";
    }

    auto conn_st = connect(
        sock, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)
    );
    warn_status(conn_st, "connection to server failed");
    if (conn_st < 0) {
        debug_status(close(sock), "close socket failed");
        return "";
    }

    debug_status(
        send(sock, request.c_str(), request.size(), 0), "send request"
    );

    std::ostringstream response;
    recv_all(sock, response);

    debug_status(close(sock), "close error");

    return response.str();
}
