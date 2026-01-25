#include "websocket_server.h"
#include <algorithm>

// WsSession implementation

WsSession::WsSession(boost::asio::io_context& io_context,
                    std::shared_ptr<WebSocketServer> server)
    : socket_(io_context),
      ws_(socket_),
      server_(server),
      last_activity_(std::chrono::steady_clock::now()),
      session_id_(next_session_id_++) {
    
    ws_.set_option(
        websocket::stream_base::decorator(
            [](websocket::request_type& req) {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-server");
            }));
}

tcp::socket& WsSession::socket() {
    return socket_;
}

void WsSession::start() {
    auto self(shared_from_this());
    ws_.async_accept(
        [this, self](const boost::system::error_code& ec) {
            if (!ec) {
                server_->register_client(self);
                last_activity_ = std::chrono::steady_clock::now();
                start_read();
            } else {
                log_error("WebSocket handshake error: " + ec.message());
            }
        });
}

void WsSession::send_message(const std::string& message) {
    try {
        ws_.write(boost::asio::buffer(message));
    } catch (const std::exception& e) {
        log_error("WebSocket send error: " + std::string(e.what()));
        close_connection();
    }
}

void WsSession::send_message_async(const std::string& message) {
    auto self(shared_from_this());
    auto buffer = std::make_shared<std::string>(message);
    
    ws_.async_write(
        boost::asio::buffer(*buffer),
        [this, self, buffer](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            if (ec) {
                log_error("WebSocket async send error: " + ec.message());
                close_connection();
            } else {
                log_info("WebSocket message sent to client " + std::to_string(session_id_) + 
                        " (" + std::to_string(bytes_transferred) + " bytes)");
            }
        });
}

bool WsSession::check_keepalive_timeout() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - last_activity_);

    // Timeout threshold: 30 seconds
    if (elapsed.count() > 30) {
        log_warn("WebSocket client " + std::to_string(session_id_) +
                " timeout (inactive for " + std::to_string(elapsed.count()) + "s)");
        return true;
    }

    // Send ping every 10 seconds
    if (elapsed.count() >= 10) {
        try {
            ws_.ping(websocket::ping_data());
            last_activity_ = now;
            log_info("WebSocket ping sent to client " + std::to_string(session_id_));
        } catch (const std::exception& e) {
            log_error("WebSocket ping error: " + std::string(e.what()));
            return true;
        }
    }

    return false;
}

void WsSession::start_read() {
    auto self(shared_from_this());
    ws_.async_read(
        buffer_,
        [this, self](const boost::system::error_code& ec,
                     std::size_t bytes_transferred) {
            if (!ec) {
                last_activity_ = std::chrono::steady_clock::now();

                if (ws_.got_text()) {
                    std::string message = beast::buffers_to_string(
                        buffer_.data());
                    buffer_.consume(bytes_transferred);
                    log_info("WebSocket message from client " +
                            std::to_string(session_id_) + ": " +
                            message.substr(0, 50));
                }
                start_read();
            } else if (ec != websocket::error::closed) {
                log_error("WebSocket read error: " + ec.message());
                close_connection();
            }
        });
}

void WsSession::close_connection() {
    try {
        ws_.close(websocket::close_code::normal);
    } catch (...) {
    }
    try {
        socket_.close();
    } catch (...) {
    }
    server_->unregister_client(shared_from_this());
    log_info("WebSocket client " + std::to_string(session_id_) + " closed");
}

// WebSocketServer implementation

WebSocketServer::WebSocketServer(boost::asio::io_context& io_context, unsigned short port)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    log_info("WebSocket Server initialized on port " + std::to_string(port));
}

WebSocketServer::~WebSocketServer() {
    try {
        acceptor_.close();
    } catch (...) {
    }
}

size_t WebSocketServer::client_count() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_.size();
}

void WebSocketServer::start() {
    start_accept();
}

void WebSocketServer::broadcast_pending_events() {
    Event event;
    int event_count = 0;
    while (get_event_manager().get_next_event(event)) {
        event_count++;
        std::string event_json = event.to_string();
        log_info("Broadcasting event to WebSocket clients: " + event.type);

        std::lock_guard<std::mutex> lock(clients_mutex_);
        int client_count = clients_.size();
        log_info("Current WebSocket client count: " + std::to_string(client_count));
        
        if (client_count == 0) {
            log_warn("No WebSocket clients connected to receive event!");
        }
        
        for (auto client : clients_) {
            log_info("Sending message to client...");
            client->send_message_async(event_json);  // Use async version
        }
    }
    
    if (event_count == 0) {
        // No events to broadcast - this is normal, just debug info
        // log_info("No pending events to broadcast");
    }
}

void WebSocketServer::register_client(std::shared_ptr<WsSession> client) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    clients_.push_back(client);
    log_info("Client registered. Total clients: " + std::to_string(clients_.size()));
}

void WebSocketServer::unregister_client(std::shared_ptr<WsSession> client) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    clients_.erase(
        std::remove_if(clients_.begin(), clients_.end(),
            [client](const std::shared_ptr<WsSession>& c) { return c == client; }),
        clients_.end());
    log_info("Client unregistered. Total clients: " + std::to_string(clients_.size()));
}

void WebSocketServer::start_accept() {
    auto new_session = std::make_shared<WsSession>(io_context_, shared_from_this());
    acceptor_.async_accept(
        new_session->socket(),
        [this, new_session](const boost::system::error_code& ec) {
            if (!ec) {
                new_session->start();
                log_info("WebSocket: New connection accepted");
            } else {
                log_error("WebSocket accept error: " + ec.message());
            }
            start_accept();
        });
}
