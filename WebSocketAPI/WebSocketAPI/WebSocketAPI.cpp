#include "common.h"
#include "event_manager.h"
#include "rest_api_server.h"
#include "websocket_server.h"
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

using boost::asio::ip::tcp;

// Configuration
constexpr unsigned short REST_API_PORT = 8080;
constexpr unsigned short WEBSOCKET_PORT = 8081;

std::atomic<bool> should_exit(false);

/**
 * REST API server thread function
 */
void run_rest_api_server() {
    try {
        boost::asio::io_context io_context;
        RestApiServer server(io_context, REST_API_PORT);
        log_info("REST API server started on port " + std::to_string(REST_API_PORT));

        while (!should_exit) {
            io_context.run_one();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        log_info("REST API server shutdown");
    } catch (const std::exception& e) {
        log_error("REST API server error: " + std::string(e.what()));
    }
}

/**
 * WebSocket server thread function
 */
void run_websocket_server() {
    try {
        boost::asio::io_context io_context;
        auto server = std::make_shared<WebSocketServer>(io_context, WEBSOCKET_PORT);
        server->start();  // Start accepting connections
        log_info("WebSocket server started on port " + std::to_string(WEBSOCKET_PORT));

        // KeepAlive check timer
        auto last_keepalive_check = std::chrono::steady_clock::now();
        auto last_broadcast_check = std::chrono::steady_clock::now();

        while (!should_exit) {
            // Run Asio operations (with timeout)
            io_context.run_one_for(std::chrono::milliseconds(10));

            // Check KeepAlive timeouts (every 2 seconds)
            auto now = std::chrono::steady_clock::now();
            auto elapsed_keepalive = std::chrono::duration_cast<std::chrono::seconds>(
                now - last_keepalive_check);
            if (elapsed_keepalive.count() >= 2) {
                // KeepAlive checks are done per-client in async callbacks
                last_keepalive_check = now;
            }

            // Broadcast pending events (frequently)
            auto elapsed_broadcast = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - last_broadcast_check);
            if (elapsed_broadcast.count() >= 50) {  // Every 50ms
                server->broadcast_pending_events();
                last_broadcast_check = now;
            }
        }

        log_info("WebSocket server shutdown");
    } catch (const std::exception& e) {
        log_error("WebSocket server error: " + std::string(e.what()));
    }
}

/**
 * Main function - Starts servers and monitors system
 */
int main() {
    try {
        // Initialize logging
        init_logger("logging_config.json");
        
        log_info("=== WebSocket API Server Starting ===");
        log_info("REST API: http://localhost:" + std::to_string(REST_API_PORT));
        log_info("WebSocket: ws://localhost:" + std::to_string(WEBSOCKET_PORT));

        // Start REST API server thread
        std::thread rest_thread(run_rest_api_server);

        // Start WebSocket server thread
        std::thread ws_thread(run_websocket_server);

        // Main thread - monitor and handle shutdown
        log_info("Servers running. Press 'q' to quit...");
        std::string input;
        while (std::getline(std::cin, input)) {
            if (input == "q" || input == "Q") {
                log_info("Shutdown signal received");
                should_exit = true;
                break;
            } else if (input == "status" || input == "s") {
                // Print current status
                auto& event_manager = get_event_manager();
                std::cout << "\n=== System Status ===" << std::endl;
                std::cout << "Pending events: " << (event_manager.has_events() ? "Yes" : "No")
                          << std::endl;
                std::cout << "Commands: 's' for status, 'q' to quit" << std::endl;
                std::cout << "==================\n" << std::endl;
            } else if (!input.empty()) {
                log_info("Unknown command: " + input);
            }
        }

        // Wait for threads to finish
        log_info("Waiting for servers to shutdown...");
        rest_thread.join();
        ws_thread.join();

        log_info("=== WebSocket API Server Stopped ===");
        return 0;
    } catch (const std::exception& e) {
        log_error("Fatal error: " + std::string(e.what()));
        return 1;
    }
}
