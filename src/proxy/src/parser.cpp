#include "parser.hpp"

#include <spdlog/spdlog.h>

#include "picohttpparser.h"

int parse_request(
    const char *request,
    size_t request_len,
    const char **method,
    size_t *method_len,
    const char **host,
    size_t *host_len
) {
    char *path;
    std::vector<phr_header> headers(100);
    size_t path_len;
    size_t num_headers = 100;
    int minor_version;
    int pret = phr_parse_request(
        request,
        request_len,
        method,
        method_len,
        (const char **)&path,
        &path_len,
        &minor_version,
        headers.data(),
        &num_headers,
        0
    );
    if (pret == -2) {
        spdlog::error("Request parsing error: request is partial");
        return PARSING_STATUS_ERROR;
    }
    if (pret == -1) {
        spdlog::error("Request parsing error: failed");
        return PARSING_STATUS_ERROR;
    }
    for (auto &header : headers) {
        if (strncmp(header.name, "Host", 4) == 0) {
            *host     = header.value;
            *host_len = header.value_len;
            break;
        }
    }
    if (host == nullptr) {
        spdlog::error("Request parsing error: host header not found");
        return PARSING_STATUS_ERROR;
    }
    return PARSING_STATUS_SUCCESS;
}
