#include "logger.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using json = nlohmann::json;

static std::shared_ptr<spdlog::logger> g_logger;

void init_logger(const std::string& config_file) {
    try {
        // Read configuration file
        std::ifstream config_stream(config_file);
        json config;
        
        if (config_stream.good()) {
            config_stream >> config;
            config_stream.close();
        } else {
            // Use default configuration
            config = json::parse(R"({
                "logging": {
                    "level": "info",
                    "console_enabled": true,
                    "file_enabled": true,
                    "file_path": "logs/websocket_api.log",
                    "max_file_size": 10485760,
                    "max_files": 5,
                    "pattern": "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
                }
            })");
        }

        auto log_config = config["logging"];
        
        // Parse log level
        auto level_str = log_config["level"].get<std::string>();
        spdlog::level::level_enum level = spdlog::level::info;
        if (level_str == "debug") level = spdlog::level::debug;
        else if (level_str == "info") level = spdlog::level::info;
        else if (level_str == "warn") level = spdlog::level::warn;
        else if (level_str == "err") level = spdlog::level::err;
        else if (level_str == "critical") level = spdlog::level::critical;

        std::vector<spdlog::sink_ptr> sinks;

        // Console sink
        if (log_config["console_enabled"].get<bool>()) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(level);
            sinks.push_back(console_sink);
        }

        // File sink (rotating)
        if (log_config["file_enabled"].get<bool>()) {
            auto file_path = log_config["file_path"].get<std::string>();
            auto max_size = log_config["max_file_size"].get<size_t>();
            auto max_files = log_config["max_files"].get<size_t>();
            
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                file_path, max_size, max_files);
            file_sink->set_level(level);
            sinks.push_back(file_sink);
        }

        // Create logger
        g_logger = std::make_shared<spdlog::logger>("websocket_api", 
                                                     sinks.begin(), sinks.end());
        g_logger->set_level(level);
        
        // Set pattern
        auto pattern = log_config["pattern"].get<std::string>();
        g_logger->set_pattern(pattern);
        
        // Register logger
        spdlog::register_logger(g_logger);
        
        log_info("Logger initialized successfully");
    } catch (const std::exception& e) {
        // Fallback: create simple console logger
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        g_logger = std::make_shared<spdlog::logger>("websocket_api", console_sink);
        g_logger->set_level(spdlog::level::info);
        g_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        spdlog::register_logger(g_logger);
        
        log_error(std::string("Failed to initialize logger: ") + e.what());
    }
}

std::shared_ptr<spdlog::logger> get_logger() {
    if (!g_logger) {
        init_logger();
    }
    return g_logger;
}

void log_info(const std::string& message) {
    if (g_logger) {
        g_logger->info(message);
    }
}

void log_error(const std::string& message) {
    if (g_logger) {
        g_logger->error(message);
    }
}

void log_warn(const std::string& message) {
    if (g_logger) {
        g_logger->warn(message);
    }
}
