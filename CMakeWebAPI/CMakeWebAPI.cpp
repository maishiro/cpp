#include "CMakeWebAPI.h"
#include "ProductController.hpp"

int main()
{
    using namespace httplib;

    Server svr;

    // コントローラーを生成してルーティングをバインド
    ProductController productCtrl;
    productCtrl.registerRoutes(svr);

    std::cout << "Server starting at http://0.0.0.0:5001..." << std::endl;
    svr.listen( "0.0.0.0", 5001 );
}
