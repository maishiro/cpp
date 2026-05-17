#include "DatabaseManager.h"
#include <fstream>
#include <iostream>
#include <soci/postgresql/soci-postgresql.h>
#include <soci/odbc/soci-odbc.h>
// Oracle backend might need specific headers if used directly, 
// but usually soci::session(backend, conn_str) is enough.
//#include <soci/oracle/soci-oracle.h>

using json = nlohmann::json;

DatabaseManager::DatabaseManager(const std::string& configPath) 
    : configPath_(configPath) {
    loadConfig();
}

void DatabaseManager::loadConfig() {
    std::ifstream f(configPath_);
    if (!f.is_open()) {
        throw std::runtime_error("Could not open config file: " + configPath_);
    }
    json data = json::parse(f);
    dbType_ = data.at("db_type").get<std::string>();
    connectionString_ = data.at("connection_string").get<std::string>();
}

void DatabaseManager::connect() {
    try {
        if (dbType_ == "postgresql") {
            session_ = std::make_unique<soci::session>(soci::postgresql, connectionString_);
        } else if (dbType_ == "sqlserver" || dbType_ == "odbc") {
            session_ = std::make_unique<soci::session>(soci::odbc, connectionString_);
        //} else if (dbType_ == "oracle") {
        //    session_ = std::make_unique<soci::session>(soci::oracle, connectionString_);
        } else {
            throw std::runtime_error("Unsupported database type: " + dbType_);
        }
        std::cout << "Connected to " << dbType_ << " successfully." << std::endl;
    } catch (const soci::soci_error& e) {
        throw std::runtime_error("SOCI Error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("Connection Error: " + std::string(e.what()));
    }
}
