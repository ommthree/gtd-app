#ifndef DATABASE_REGISTRY_H
#define DATABASE_REGISTRY_H

#include <variant>
#include <vector>
#include <map>
#include <string>
#include <mysql.h>
#include <sqlite3.h>

// Enum to distinguish between MySQL and SQLite connections
enum class DatabaseType {
    MYSQL,
    SQLITE
};

// Connection object wrapping a MySQL or SQLite connection
struct DatabaseConnection {
    DatabaseType type;
    std::variant<MYSQL*, sqlite3*> connection;
};

// === Global Registry ===

// All database connections (indexed by DB ID)
extern std::vector<DatabaseConnection> allDatabases;

// Mapping of table name → list of DB IDs it lives in (e.g. "Tasks": [0,1])
extern std::map<std::string, std::vector<int>> tableToDatabaseIds;

// === Loaders ===

// Load database connection info from a JSON file
bool loadDatabaseConfigs(const std::string& configPath);

// Load table-to-database mappings from a JSON file
bool loadTableMappings(const std::string& mappingPath);

// Human-readable names of databases, e.g., ["Shared DB", "Work DB"]
extern std::vector<std::string> databaseNames;

#endif // DATABASE_REGISTRY_H