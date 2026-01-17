#include <iostream>
#include "httplib.h"
#include <nlohmann/json.hpp>

#pragma comment(lib, "cpp-httplib.lib")

using namespace std;
using json = nlohmann::json;


int main()
{
	// HTTP
	httplib::Server svr;

	svr.Get( "/weather", []( const httplib::Request&, httplib::Response& res ) {
		cout << "GET /weather request." << endl;

		json j;
		j["locale"] = "Tokyo";
		j["temperature"] = 23.1;

		res.set_content( j.dump(), "application/json" );
	} );


	svr.Post( "/echo", []( const httplib::Request& req, httplib::Response& res ) {
		cout << "POST /echo request." << endl;

		json data = json::parse( req.body );
		cout << data.dump( 2 ) << endl;

		res.set_content( req.body, "application/json" );
	} );


	svr.listen( "0.0.0.0", 8888 );
	
	return 0;
}
