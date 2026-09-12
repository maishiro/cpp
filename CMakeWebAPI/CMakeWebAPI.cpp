#include "CMakeWebAPI.h"
#include "httplib.h"

int main()
{
    using namespace httplib;

    Server svr;

    svr.Get( "/hi", []( const Request& req, Response& res ) {
        std::cout << "GET /hi " << "Request from: " << req.remote_addr << ":" << req.remote_port << std::endl;
        res.set_content( "Hello World!", "text/plain" );
    } );

    // Match the request path against a regular expression
    // and extract its captures
    svr.Get( R"(/numbers/(\d+))", [&]( const Request& req, Response& res ) {
        auto numbers = req.matches[1];
        std::cout << "GET /numbers/" << numbers << " " << "Request from: " << req.remote_addr << ":" << req.remote_port << std::endl;
        res.set_content( numbers, "text/plain" );
    } );

    // Capture the second segment of the request path as "id" path param
    svr.Get( "/users/:id", [&]( const Request& req, Response& res ) {
        auto user_id = req.path_params.at( "id" );
        std::cout << "GET /users/" << user_id << " " << "Request from: " << req.remote_addr << ":" << req.remote_port << std::endl;
        res.set_content( user_id, "text/plain" );
    } );

    // Extract values from HTTP headers and URL query params
    svr.Get( "/body-header-param", []( const Request& req, Response& res ) {
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
    } );

    std::cout << "Server starting at http://0.0.0.0:5001..." << std::endl;
    svr.listen( "0.0.0.0", 5001 );
}
