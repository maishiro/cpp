#include <iostream>
#include "DatabaseManager.h"

int main() {
    try {
        DatabaseManager dbManager("config.json");
        dbManager.connect();

        soci::session& sql = dbManager.getSession();

        // サンプルクエリ（テーブルが存在する場合）
        // int count;
        // sql << "select count(*) from some_table", soci::into(count);
        // std::cout << "Count: " << count << std::endl;

        std::cout << "Application finished successfully." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
