#pragma once

#include <string>

std::string
forward_request(const std::string &host, const std::string &request);

/*!
 * \brief handle status
 * log error if status != 0
 */
void hs(int status, const std::string &msg);
