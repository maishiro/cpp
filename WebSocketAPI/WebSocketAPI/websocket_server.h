#pragma once

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#include "common.h"
#include "event_manager.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <vector>
#include <mutex>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
using boost::asio::ip::tcp;

// Forward declaration
class WebSocketServer;

/**
 * Represents a single WebSocket client session
 */
class WsSession : public std::enable_shared_from_this<WsSession> {
public:
    explicit WsSession(boost::asio::io_context& io_context,
                      std::shared_ptr<WebSocketServer> server);

    tcp::socket& socket();
    void start();
    void send_message(const std::string& message);
    void send_message_async(const std::string& message);
    bool check_keepalive_timeout();

private:
    static inline int next_session_id_ = 1;
    int session_id_;
    tcp::socket socket_;
    websocket::stream<tcp::socket&> ws_;
    std::shared_ptr<WebSocketServer> server_;
    beast::flat_buffer buffer_;
    std::chrono::steady_clock::time_point last_activity_;

    void start_read();
    void close_connection();

    friend class WebSocketServer;
};

/**
 * WebSocket Server with KeepAlive mechanism
 */
class WebSocketServer : public std::enable_shared_from_this<WebSocketServer> {
public:
    WebSocketServer(boost::asio::io_context& io_context, unsigned short port);
    ~WebSocketServer();

    void start();  // Must be called after construction
    size_t client_count() const;
    void broadcast_pending_events();
    void register_client(std::shared_ptr<WsSession> client);
    void unregister_client(std::shared_ptr<WsSession> client);

private:
    boost::asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<WsSession>> clients_;
    mutable std::mutex clients_mutex_;

    void start_accept();

    friend class WsSession;
};
