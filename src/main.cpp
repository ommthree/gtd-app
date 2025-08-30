#include "core/database.h"
#include "core/config.h"
#include "platform/windows/gui_win32.h"
#include "ui/lookup_maps.h"
#include "core/database_registry.h"

#include <mysql.h>
#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <exception>

int main() {
    try {
        std::cout << "[START] main()\n";

        // === Load database connections ===
        std::cout << "Loading database configs from: " << AppConfig::kDatabaseConfigPath << "\n";
        if (!loadDatabaseConfigs(AppConfig::kDatabaseConfigPath)) {
            std::cerr << " Failed to load database configs.\n";
            return 1;
        }

        // === Load table mappings ===
        std::cout << "Loading table mappings from: " << AppConfig::kTableMappingPath << "\n";
        if (!loadTableMappings(AppConfig::kTableMappingPath)) {
            std::cerr << " Failed to load table mappings.\n";
            return 1;
        }

        std::cout << "[OK] Database configs and table mappings loaded.\n";

        // === Identify shared MySQL DB for lookups ===
        std::cout << "Determining shared MySQL DB for lookups...\n";
        int sharedDbId = -1;
        if (tableToDatabaseIds.count("Projects") && !tableToDatabaseIds["Projects"].empty()) {
            sharedDbId = tableToDatabaseIds["Projects"][0];
        }

        if (sharedDbId >= 0 && sharedDbId < allDatabases.size()) {
            if (allDatabases[sharedDbId].type == DatabaseType::MYSQL) {
                MYSQL* conn = std::get<MYSQL*>(allDatabases[sharedDbId].connection);
                std::cout << "Calling populateLookupMaps(conn)...\n";
                populateLookupMaps(conn);
                std::cout << "[OK] Lookup tables populated.\n";
            }
            else {
                std::cerr << "Expected MySQL for lookup tables but got SQLite.\n";
            }
        }
        else {
            std::cerr << "Could not determine shared DB ID for lookups.\n";
        }

        // === Load tasks from all databases ===
        std::cout << "Fetching tasks from all databases...\n";
        std::vector<Task> tasks = fetchTasksFromDatabase();
        std::cout << "[OK] Fetched " << tasks.size() << " tasks.\n";

        // === Launch GUI ===
        std::cout << "Launching GUI...\n";
        launch_gui(tasks);
        std::cout << "[OK] GUI closed.\n";

        // === Cleanup ===
        std::cout << "Cleaning up...\n";
        for (auto& db : allDatabases) {
            if (db.type == DatabaseType::MYSQL) {
                mysql_close(std::get<MYSQL*>(db.connection));
            }
            else if (db.type == DatabaseType::SQLITE) {
                sqlite3_close(std::get<sqlite3*>(db.connection));
            }
        }
        std::cout << "[END] main() finished cleanly.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Unhandled exception : " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unhandled unknown exception.\n";
    }

    return 0;
}