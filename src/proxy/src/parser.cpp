#include "parser.hpp"

#include <netdb.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>

#include "picohttpparser.h"
#include "utils.hpp"

bool parse_request(
    const char *buf,
    ssize_t len,
    std::string &method,
    std::string &url,
    std::string &host
) {
    const char *method_ptr;
    const char *path_ptr;
    size_t method_len;
    size_t path_len;
    size_t num_headers = 100;
    std::vector<phr_header> headers(num_headers);
    int minor_version;

    int pret = phr_parse_request(
        buf,
        len,
        &method_ptr,
        &method_len,
        &path_ptr,
        &path_len,
        &minor_version,
        headers.data(),
        &num_headers,
        0
    );

    if (pret <= 0) {
        spdlog::warn("cant parse request");
        return false;
    }

    method = std::string(method_ptr, method_len);
    url    = std::string(path_ptr, path_len);

    for (size_t i = 0; i < num_headers; ++i) {
        if (strncasecmp(headers[i].name, "Host", headers[i].name_len) == 0) {
            host = std::string(headers[i].value, headers[i].value_len);
        }
    }

    return true;
}

bool resolve_host(const std::string &host, std::string &ip_address, int &port) {
    if (is_url(host)) {
        return false;
    }
    port = 80;
    struct addrinfo hints {};
    struct addrinfo *res;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    auto host_string = host;

    if (is_localhost_url(host)) {
        host_string    = "127.0.0.1";
        auto colon_pos = host.find(':', 6);    // отступаем 6 символов, чтобы не
                                               // найти : в http:// или https://
        if (colon_pos != std::string::npos) {
            std::string port_substring = host.substr(colon_pos + 1);
            std::string port_str;
            for (int j = 0; j < 5; ++j) {    // считваем порт. 5 потому что
                                             // максимальный порт 65535
                spdlog::trace("port_substring[{}] = {}", j, port_substring[j]);
                if (port_substring[j] < '0' || port_substring[j] > '9') {
                    break;
                }
                port_str += port_substring[j];
            }
            try {
                port = std::stoi(port_str);
            }
            catch (const std::exception &) {
                spdlog::error("{} is not a port", port_str);
                return false;
            }
        }
    }

    if (getaddrinfo(host_string.c_str(), nullptr, &hints, &res) != 0) {
        spdlog::error("getaddrinfo");
        return false;
    }
    std::array<char, INET_ADDRSTRLEN> ip {};
    if (inet_ntop(
            AF_INET,
            &((sockaddr_in *)res->ai_addr)->sin_addr,
            ip.data(),
            INET_ADDRSTRLEN
        )
        == nullptr) {
        spdlog::error("inet_ntop");
        freeaddrinfo(res);
        return false;
    }
    ip_address = ip.data();
    freeaddrinfo(res);
    return true;
}
