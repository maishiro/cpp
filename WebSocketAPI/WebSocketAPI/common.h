#pragma once

#include <string>
#include <iostream>
#include <chrono>
#include <memory>
#include <nlohmann/json.hpp>
#include "logger.h"

// Suppress Windows deprecation warnings
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

using json = nlohmann::json;

// Event structure for broadcasting
struct Event {
    std::string type;          // Event type (e.g., "user_action", "system_alert")
    std::string timestamp;     // ISO 8601 timestamp
    json payload;              // Event payload as JSON

    json to_json() const;
    std::string to_string() const;
};

// Get current timestamp in ISO 8601 format
std::string get_iso8601_timestamp();
