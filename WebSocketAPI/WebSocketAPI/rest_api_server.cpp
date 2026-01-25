#include "rest_api_server.h"
#include <sstream>
#include <algorithm>

RestApiServer::RestApiServer(boost::asio::io_context& io_context, unsigned short port)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    log_info("REST API Server initialized on port " + std::to_string(port));
    start_accept();
}

RestApiServer::~RestApiServer() {
    try {
        acceptor_.close();
    } catch (...) {
    }
}

void RestApiServer::start_accept() {
    auto new_session = std::make_shared<HttpSession>(io_context_);
    acceptor_.async_accept(
        new_session->socket(),
        [this, new_session](const boost::system::error_code& ec) {
            if (!ec) {
                new_session->start();
                log_info("REST API: New connection accepted");
            } else {
                log_error("REST API accept error: " + ec.message());
            }
            start_accept();
        });
}

RestApiServer::HttpSession::HttpSession(boost::asio::io_context& io_context)
    : socket_(io_context) {}

tcp::socket& RestApiServer::HttpSession::socket() {
    return socket_;
}

void RestApiServer::HttpSession::start() {
    auto self(shared_from_this());
    
    // Read data into our buffer
    auto read_buffer = std::make_shared<std::vector<char>>(8192);
    
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(*read_buffer),
        boost::asio::transfer_at_least(1),
        [this, self, read_buffer](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            if (!ec && bytes_transferred > 0) {
                log_info("REST API: Read " + std::to_string(bytes_transferred) + " bytes");
                // Append to buffer
                buffer_.insert(buffer_.end(), read_buffer->begin(), read_buffer->begin() + bytes_transferred);
                handle_request();
            } else if (ec != boost::asio::error::eof) {
                log_error("REST API read error: " + ec.message());
            }
        });
}

void RestApiServer::HttpSession::read_request_body() {
    auto self(shared_from_this());
    
    // Read more data into our buffer
    auto read_buffer = std::make_shared<std::vector<char>>(8192);
    
    boost::asio::async_read(
        socket_,
        boost::asio::buffer(*read_buffer),
        boost::asio::transfer_at_least(1),
        [this, self, read_buffer](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            if (!ec && bytes_transferred > 0) {
                log_info("REST API: Read more " + std::to_string(bytes_transferred) + " bytes");
                // Append to buffer
                buffer_.insert(buffer_.end(), read_buffer->begin(), read_buffer->begin() + bytes_transferred);
                handle_request();
            } else {
                log_error("REST API body read error: " + (ec ? ec.message() : "eof"));
            }
        });
}

void RestApiServer::HttpSession::handle_request() {
    try {
        if (request_handled) {
            return;  // Already handled
        }
        
        size_t buffer_size = buffer_.size();
        log_info("REST API: Buffer has " + std::to_string(buffer_size) + " bytes");
        
        if (buffer_size == 0) {
            log_warn("REST API: Buffer is empty, reading more data...");
            read_request_body();
            return;
        }
        
        // Find header/body boundary
        std::string buffer_str(buffer_.begin(), buffer_.end());
        size_t header_end = buffer_str.find("\r\n\r\n");
        
        if (header_end == std::string::npos) {
            log_warn("REST API: Headers not complete yet, reading more data...");
            read_request_body();
            return;
        }
        
        header_end += 4;  // Include the \r\n\r\n
        
        // Extract Content-Length from headers (case-insensitive)
        size_t content_length = 0;
        std::string buffer_lower = buffer_str;
        std::transform(buffer_lower.begin(), buffer_lower.end(), buffer_lower.begin(), ::tolower);
        
        size_t cl_pos = buffer_lower.find("content-length:");
        if (cl_pos != std::string::npos) {
            size_t cl_end = buffer_str.find("\r\n", cl_pos);
            std::string cl_str = buffer_str.substr(cl_pos + 15, cl_end - cl_pos - 15);
            // Trim whitespace
            cl_str.erase(0, cl_str.find_first_not_of(" \t"));
            cl_str.erase(cl_str.find_last_not_of(" \t") + 1);
            try {
                content_length = std::stoul(cl_str);
                log_info("REST API: Content-Length = " + std::to_string(content_length));
            } catch (...) {
                log_warn("REST API: Could not parse Content-Length value: " + cl_str);
            }
        } else {
            log_info("REST API: No Content-Length header found");
        }
        
        size_t total_needed = header_end + content_length;
        log_info("REST API: Header ends at " + std::to_string(header_end) + 
                ", total needed = " + std::to_string(total_needed) + 
                ", have " + std::to_string(buffer_size));
        
        if (buffer_size < total_needed) {
            log_warn("REST API: Incomplete message, reading more...");
            read_request_body();
            return;
        }
        
        // Extract method, target, and body manually
        size_t first_space = buffer_str.find(' ');
        size_t second_space = buffer_str.find(' ', first_space + 1);
        
        std::string method = buffer_str.substr(0, first_space);
        std::string target = buffer_str.substr(first_space + 1, second_space - first_space - 1);
        std::string body = buffer_str.substr(header_end, content_length);
        
        log_info("REST API: " + method + " " + target);
        log_info("REST API: Body length=" + std::to_string(body.length()));
        if (!body.empty()) {
            log_info("REST API: Body=" + body.substr(0, 300));
        }
        
        request_handled = true;
        
        // Route the request
        if (method == "GET" && target == "/") {
            send_response(200, "WebSocket API Server is running");
        } else if (method == "POST" && target == "/api/event") {
            if (body.empty()) {
                log_warn("POST /api/event received empty body");
                send_json_response(400, std::string("Request body is empty"));
            } else {
                handle_post_event(body);
            }
        } else {
            send_response(404, "Not Found");
        }
    } catch (const std::exception& e) {
        log_error("REST API request handling error: " + std::string(e.what()));
        send_response(400, "Bad Request");
    }
}

void RestApiServer::HttpSession::handle_post_event(const std::string& body) {
    try {
        // Empty body check
        if (body.empty()) {
            log_error("POST /api/event received empty body");
            send_json_response(400, std::string("Request body is empty"));
            return;
        }

        log_info("REST API: Received body: " + body.substr(0, 100) + 
                (body.length() > 100 ? "..." : ""));

        auto request_json = json::parse(body);

        // Validate request format
        if (!request_json.contains("type") || 
            !request_json.contains("data")) {
            send_json_response(400, std::string("Missing 'type' or 'data' field"));
            return;
        }

        // Create and publish event
        Event event;
        event.type = request_json["type"].get<std::string>();
        event.timestamp = get_iso8601_timestamp();
        event.payload = request_json["data"];

        get_event_manager().publish_event(event);

        // Send success response
        json response;
        response["status"] = "success";
        response["message"] = "Event received and queued for broadcast";
        response["event_type"] = event.type;
        response["timestamp"] = event.timestamp;

        send_json_response(200, response);
        log_info("REST API: /api/event handled successfully");
    } catch (const json::parse_error& e) {
        log_error("JSON parse error: " + std::string(e.what()));
        send_json_response(400, std::string("Invalid JSON format: ") + e.what());
    } catch (const std::exception& e) {
        log_error("Event handling error: " + std::string(e.what()));
        send_json_response(500, std::string("Internal server error"));
    }
}

void RestApiServer::HttpSession::send_json_response(int status_code, const json& response_body) {
    std::string body = response_body.dump();
    send_response(status_code, body, "application/json");
}

void RestApiServer::HttpSession::send_json_response(int status_code, const std::string& message) {
    json response;
    response["status"] = (status_code == 200) ? "success" : "error";
    response["message"] = message;
    send_json_response(status_code, response);
}

void RestApiServer::HttpSession::send_response(int status_code, const std::string& body,
                      const std::string& content_type) {
    try {
        std::ostringstream response;

        // Status line
        response << "HTTP/1.1 " << status_code << " ";
        switch (status_code) {
            case 200: response << "OK"; break;
            case 400: response << "Bad Request"; break;
            case 404: response << "Not Found"; break;
            case 500: response << "Internal Server Error"; break;
            default: response << "Error"; break;
        }
        response << "\r\n";

        // Headers
        response << "Content-Type: " << content_type << "\r\n";
        response << "Content-Length: " << body.size() << "\r\n";
        response << "Connection: close\r\n";
        response << "\r\n";

        // Body
        response << body;

        // Send response
        auto response_str = response.str();
        boost::asio::write(socket_,
            boost::asio::buffer(response_str));

        log_info("REST API: Response sent (status=" + std::to_string(status_code) + ")");
    } catch (const std::exception& e) {
        log_error("Response sending error: " + std::string(e.what()));
    }

    // Close connection
    socket_.close();
}
