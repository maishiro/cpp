#pragma once
#include <soci/soci.h>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

class DatabaseManager {
public:
    DatabaseManager(const std::string& configPath);
    
    // soci::sessionを取得
    soci::session& getSession() { return *session_; }

    void connect();

private:
    std::string configPath_;
    std::string dbType_;
    std::string connectionString_;
    std::unique_ptr<soci::session> session_;

    void loadConfig();
};
