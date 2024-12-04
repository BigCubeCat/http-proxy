#pragma once

#include <cstddef>
#include <string>


const int PARSING_STATUS_SUCCESS   = 0;
const int PARSING_STATUS_ERROR     = -1;
const int PARSING_STATUS_NO_CLIENT = -2;


static int parse_request(
    const std::string &request,
    size_t request_len,
    const char **method,
    size_t *method_len,
    const char **host,
    size_t *host_len
);
