#include "database_registry.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

// Global list of all database connections
std::vector<DatabaseConnection> allDatabases;

// Mapping from table name → vector of DB IDs (to allow multiple DBs per table)
std::map<std::string, std::vector<int>> tableToDatabaseIds;

// Global list of database names (for UI dropdown etc.)
std::vector<std::string> databaseNames;

bool loadDatabaseConfigs(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << " Failed to open database config file: " << configPath << "\n";
        return false;
    }

    json dbConfig;
    try {
        file >> dbConfig;
    }
    catch (const std::exception& e) {
        std::cerr << " Failed to parse database config JSON: " << e.what() << "\n";
        return false;
    }

    for (const auto& db : dbConfig) {
        DatabaseConnection conn;
        std::string type = db.value("type", "");

        if (type == "mysql") {
            MYSQL* mysql = mysql_init(nullptr);
            if (!mysql) {
                std::cerr << " mysql_init() failed.\n";
                continue;
            }

            const std::string& host = db["host"];
            const std::string& user = db["user"];
            const std::string& password = db["password"];
            const std::string& database = db["database"];
            int port = db.value("port", 3306);

            std::cout << "Connecting to MySQL at " << host << ":" << port << "...\n";
            if (!mysql_real_connect(mysql, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, nullptr, 0)) {
                std::cerr << " mysql_real_connect() failed: " << mysql_error(mysql) << "\n";
                mysql_close(mysql);
                continue;
            }

            conn.type = DatabaseType::MYSQL;
            conn.connection = mysql;
            allDatabases.push_back(conn);

            std::string label = db.value("label", "MySQL at " + host);
            databaseNames.push_back(label);
        }
        else if (type == "sqlite") {
            sqlite3* sqlite = nullptr;
            const std::string& path = db["path"];
            std::cout << "Opening SQLite at " << path << "...\n";
            if (sqlite3_open(path.c_str(), &sqlite) != SQLITE_OK) {
                std::cerr << " sqlite3_open() failed for path: " << path << "\n";
                continue;
            }

            conn.type = DatabaseType::SQLITE;
            conn.connection = sqlite;
            allDatabases.push_back(conn);

            std::string label = db.value("label", "SQLite: " + path);
            databaseNames.push_back(label);
        }
        else {
            std::cerr << " Unknown database type: " << type << "\n";
        }
    }

    std::cout << "[DEBUG] Loaded " << databaseNames.size() << " database names.\n";
    return true;
}

// === Load table_map.json ===
bool loadTableMappings(const std::string& mappingPath) {
    std::ifstream file(mappingPath);
    if (!file.is_open()) {
        std::cerr << " Failed to open table mapping file: " << mappingPath << "\n";
        return false;
    }

    json tableMap;
    try {
        file >> tableMap;
    }
    catch (const std::exception& e) {
        std::cerr << " Failed to parse table mapping JSON: " << e.what() << "\n";
        return false;
    }

    for (auto& [tableName, dbIdList] : tableMap.items()) {
        tableToDatabaseIds[tableName] = dbIdList.get<std::vector<int>>();
    }

    return true;
}