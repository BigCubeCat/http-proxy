#pragma once

#include <chrono>

std::time_t current_unixtime() {
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()
    );
}
