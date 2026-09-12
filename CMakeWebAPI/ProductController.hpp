#include "httplib.h"

class ProductController {
public:
    // Server の参照を受け取ってルーティングを一括登録する
    void registerRoutes(httplib::Server& svr) {
        svr.Get("/hi", [this](const auto& req, auto& res) {
            getHello(req, res);
        });

        // Match the request path against a regular expression
        // and extract its captures
        svr.Get(R"(/numbers/(\d+))", [this](const auto& req, auto& res) {
            getNumbers(req, res);
        });

        // Capture the second segment of the request path as "id" path param
        svr.Get("/users/:id", [this](const auto& req, auto& res) {
            getUserID(req, res);
        });

        /// Extract values from HTTP headers and URL query params
        svr.Get("/body-header-param", [this](const auto& req, auto& res) {
            getBodyHeader(req, res);
        });
    }

protected:
    virtual void getHello(const httplib::Request& req, httplib::Response& res) {
        std::cout << "GET /hi " << "Request from: " << req.remote_addr << ":" << req.remote_port << std::endl;
        res.set_content( "Hello World!", "text/plain" );
    }

    // Match the request path against a regular expression
    // and extract its captures
    virtual void getNumbers(const httplib::Request& req, httplib::Response& res) {
        auto numbers = req.matches[1];
        std::cout << "GET /numbers/" << numbers << " " << "Request from: " << req.remote_addr << ":" << req.remote_port << std::endl;
        res.set_content( numbers, "text/plain" );
    }

    // Capture the second segment of the request path as "id" path param
    virtual void getUserID(const httplib::Request& req, httplib::Response& res) {
        auto user_id = req.path_params.at( "id" );
        std::cout << "GET /users/" << user_id << " " << "Request from: " << req.remote_addr << ":" << req.remote_port << std::endl;
        res.set_content( user_id, "text/plain" );
    }

    // Extract values from HTTP headers and URL query params
    virtual void getBodyHeader(const httplib::Request& req, httplib::Response& res) {
        std::cout << "GET /body-header-param/" << " " << "Request from: " << req.remote_addr << ":" << req.remote_port << std::endl;
        std::cout << "req.headers.size() = " << req.headers.size()
                << ", req.params.size() = " << req.params.size() << std::endl;

        std::cout << "--- Received Headers ---" << std::endl;
        for (const auto& header : req.headers) {
            std::cout << header.first << ": " << header.second << std::endl;
        }
        std::cout << "------------------------" << std::endl;

        if( req.has_header( "Content-Length" ) ) {
            auto val = req.get_header_value( "Content-Length" );
            std::cout << "Content-Length = " << val << std::endl;
        }
        if( req.has_param( "opt1" ) ) {
            auto val = req.get_param_value( "opt1" );
            std::cout << "opt1 = " << val << std::endl;
        }
        // Get all values for a given key (e.g., ?tag=a&tag=b)
        auto values = req.get_param_values( "tag" );
        for( const auto& v : values ) {
            std::cout << "tag = " << v << std::endl;
        }
        res.set_content( req.body, "text/plain" );
    }
};
