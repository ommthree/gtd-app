#include "core/lookup_maps.h"
#include "core/database_registry.h"

#include <mysql.h>
#include <sqlite3.h>
#include <iostream>
#include <map>

// Global lookup maps
std::map<int, std::string> contextLookup;
std::map<int, std::string> topicLookup;
std::map<int, std::string> personLookup;
std::map<int, std::string> categoryLookup;
// Note: projectLookup now maps from string (uuid) to name
std::map<std::string, std::string> projectLookup;

void loadLookupTableFromMySQL(const std::string& table, MYSQL* conn) {
    std::string idCol = (table == "Projects") ? "uuid" : "id";
    std::string query = "SELECT " + idCol + ", name FROM " + table;

    if (mysql_query(conn, query.c_str()) != 0) {
        std::cerr << " MySQL query failed for table " << table << ": " << mysql_error(conn) << "\n";
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) return;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        if (table == "Projects") {
            std::string uuid = row[0];
            std::string name = row[1];
            projectLookup[uuid] = name;
        }
        else {
            int id = std::stoi(row[0]);
            std::string name = row[1];

            if (table == "Contexts") contextLookup[id] = name;
            else if (table == "Topics") topicLookup[id] = name;
            else if (table == "People") personLookup[id] = name;
            else if (table == "Categories") categoryLookup[id] = name;
        }
    }

    mysql_free_result(result);
}

void loadLookupTableFromSQLite(const std::string& table, sqlite3* db) {
    std::string idCol = (table == "Projects") ? "uuid" : "id";
    std::string query = "SELECT " + idCol + ", name FROM " + table;
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << " SQLite query failed for table " << table << ": " << sqlite3_errmsg(db) << "\n";
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (table == "Projects") {
            std::string uuid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            projectLookup[uuid] = name;
        }
        else {
            int id = sqlite3_column_int(stmt, 0);
            std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

            if (table == "Contexts") contextLookup[id] = name;
            else if (table == "Topics") topicLookup[id] = name;
            else if (table == "People") personLookup[id] = name;
            else if (table == "Categories") categoryLookup[id] = name;
        }
    }

    sqlite3_finalize(stmt);
}

void populateLookupMaps() {
    std::vector<std::string> tables = { "Projects", "Contexts", "Topics", "People", "Categories" };

    for (const auto& table : tables) {
        if (!tableToDatabaseIds.count(table)) continue;

        for (int dbId : tableToDatabaseIds[table]) {
            if (dbId < 0 || dbId >= allDatabases.size()) continue;

            const auto& dbConn = allDatabases[dbId];
            if (dbConn.type == DatabaseType::MYSQL) {
                loadLookupTableFromMySQL(table, std::get<MYSQL*>(dbConn.connection));
            }
            else if (dbConn.type == DatabaseType::SQLITE) {
                loadLookupTableFromSQLite(table, std::get<sqlite3*>(dbConn.connection));
            }
        }
    }

    std::cout << "[DEBUG] Lookup map sizes:\n";
    std::cout << "  Projects:   " << projectLookup.size() << "\n";
    std::cout << "  Contexts:   " << contextLookup.size() << "\n";
    std::cout << "  Topics:     " << topicLookup.size() << "\n";
    std::cout << "  People:     " << personLookup.size() << "\n";
    std::cout << "  Categories: " << categoryLookup.size() << "\n";
}