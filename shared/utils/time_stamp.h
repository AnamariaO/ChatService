#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

inline std::string currentTimestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t now_time = system_clock::to_time_t(now);
    std::tm* tm_info = std::localtime(&now_time);

    std::ostringstream oss;
    oss << std::put_time(tm_info, "[%Y-%m-%d %H:%M:%S]");

    return oss.str();
}