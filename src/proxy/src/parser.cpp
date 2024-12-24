#include "parser.hpp"

#include <netdb.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>

#include "picohttpparser.h"

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

bool resolve_host(const std::string &host, std::string &ip_address) {
    struct addrinfo hints {};
    struct addrinfo *res;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0) {
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
