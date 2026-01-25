#pragma once

#include <memory>
#include <string>
#include <spdlog/spdlog.h>

/**
 * Initialize the logger from configuration file
 * @param config_file Path to logging_config.json
 */
void init_logger(const std::string& config_file = "logging_config.json");

/**
 * Get the global logger instance
 * @return Shared pointer to spdlog logger
 */
std::shared_ptr<spdlog::logger> get_logger();

/**
 * Log info level message
 * @param message Message to log
 */
void log_info(const std::string& message);

/**
 * Log error level message
 * @param message Message to log
 */
void log_error(const std::string& message);

/**
 * Log warn level message
 * @param message Message to log
 */
void log_warn(const std::string& message);
