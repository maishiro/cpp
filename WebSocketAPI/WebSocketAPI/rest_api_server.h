#pragma once

#include "common.h"
#include "event_manager.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>

namespace beast = boost::beast;
namespace http = beast::http;
using boost::asio::ip::tcp;

/**
 * REST API Server - Handles HTTP POST requests
 * Endpoint: POST /api/event
 * Payload: JSON with "type" and "data" fields
 * Response: JSON with "status" and "message" fields
 */
class RestApiServer {
public:
    RestApiServer(boost::asio::io_context& io_context, unsigned short port);
    ~RestApiServer();

private:
    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;

    void start_accept();

    // HTTP Session to handle a single connection
    class HttpSession : public std::enable_shared_from_this<HttpSession> {
    public:
        using pointer = std::shared_ptr<HttpSession>;

        HttpSession(boost::asio::io_context& io_context);

        tcp::socket& socket();
        void start();

    private:
        tcp::socket socket_;
        std::vector<char> buffer_;
        bool request_handled = false;

        void read_request_body();
        void handle_request();
        void handle_post_event(const std::string& body);
        void send_json_response(int status_code, const json& response_body);
        void send_json_response(int status_code, const std::string& message);
        void send_response(int status_code, const std::string& body,
                          const std::string& content_type = "text/plain");
    };
};
