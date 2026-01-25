#include "common.h"
#include <ctime>

// Event implementation
json Event::to_json() const {
    return json{
        {"type", type},
        {"timestamp", timestamp},
        {"payload", payload}
    };
}

std::string Event::to_string() const {
    return to_json().dump();
}

std::string get_iso8601_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    char buffer[32];
    std::tm tm_info;
    localtime_s(&tm_info, &time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &tm_info);
    
    return std::string(buffer) + "." + std::to_string(ms.count()).substr(0, 3) + "Z";
}
