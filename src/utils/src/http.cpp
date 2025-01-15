#include "http.hpp"


void change_http_version_in_message(
    std::string &request, size_t http_ver_index, size_t http_ver_size
) {
    std::string target_http_ver = "HTTP/1.0";
    for (size_t i = 0; i < http_ver_size; ++i) {
        request[i + http_ver_index] = target_http_ver[i];
    }
    if (http_ver_size < target_http_ver.size() - 1) {
        request.insert(
            http_ver_size + http_ver_index,
            target_http_ver.data() + http_ver_size,
            target_http_ver.size() - 1 - http_ver_size
        );
    }
}
