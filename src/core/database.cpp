#include "database.h"
#include "task.h"
#include "core/database_registry.h"
#include "core/lookup_maps.h"

#include <mysql.h>
#include <sqlite3.h>
#include <vector>
#include <optional>
#include <iostream>
#include <sstream>
#include <variant>
#include <iomanip>



std::vector<Task> fetchTasksFromMySQL(MYSQL* conn, int db_id) {
    std::vector<Task> tasks;

    const char* query = R"(SELECT uuid, title, notes, category_id, context_id, 
        project_uuid, topic_id, delegated_to, time_required_minutes, 
        in_focus, due_date, defer_date, created_at, updated_at, 
        is_done, completed_at, link_from, link_to, is_locked 
        FROM Tasks)";

    if (mysql_query(conn, query) != 0) {
        std::cerr << "Query failed: " << mysql_error(conn) << "\n";
        return tasks;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "Result storage failed: " << mysql_error(conn) << "\n";
        return tasks;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        Task t;
        int i = 0;

        t.uuid = row[i++] ? row[i - 1] : "";
        t.title = row[i++] ? row[i - 1] : "";
        t.notes = row[i++] ? row[i - 1] : "";

        t.category_id = row[i] ? std::optional<int>{ std::stoi(row[i]) } : std::nullopt;
        t.category_label = (t.category_id && categoryLookup.count(*t.category_id))
            ? std::optional<std::string>{ categoryLookup[*t.category_id] } : std::nullopt;
        ++i;

        t.context_id = row[i] ? std::optional<int>{ std::stoi(row[i]) } : std::nullopt;
        t.context_label = (t.context_id && contextLookup.count(*t.context_id))
            ? std::optional<std::string>{ contextLookup[*t.context_id] } : std::nullopt;
        ++i;

        t.project_uuid = row[i] ? std::optional<std::string>{ row[i] } : std::nullopt;
        t.project_title = (t.project_uuid && projectLookup.count(*t.project_uuid))
            ? std::optional<std::string>{ projectLookup[*t.project_uuid] } : std::nullopt;
        ++i;

        t.topic_id = row[i] ? std::optional<int>{ std::stoi(row[i]) } : std::nullopt;
        t.topic_label = (t.topic_id && topicLookup.count(*t.topic_id))
            ? std::optional<std::string>{ topicLookup[*t.topic_id] } : std::nullopt;
        ++i;

        t.delegated_to = row[i] ? std::optional<int>{ std::stoi(row[i]) } : std::nullopt;
        t.delegate_name = (t.delegated_to && personLookup.count(*t.delegated_to))
            ? std::optional<std::string>{ personLookup[*t.delegated_to] } : std::nullopt;
        ++i;

        t.time_required_minutes = row[i++] ? std::optional<int>{ std::stoi(row[i - 1]) } : std::nullopt;
        t.in_focus = row[i++] ? std::stoi(row[i - 1]) != 0 : false;
        t.due_date = row[i++] ? std::optional<std::string>{ row[i - 1] } : std::nullopt;
        t.defer_date = row[i++] ? std::optional<std::string>{ row[i - 1] } : std::nullopt;
        t.created_at = row[i++] ? std::optional<std::string>{ row[i - 1] } : std::nullopt;
        t.updated_at = row[i++] ? std::optional<std::string>{ row[i - 1] } : std::nullopt;
        t.is_done = row[i++] ? std::stoi(row[i - 1]) != 0 : false;
        t.completed_at = row[i++] ? std::optional<std::string>{ row[i - 1] } : std::nullopt;
        t.link_from = row[i++] ? row[i - 1] : "";
        t.link_to = row[i++] ? row[i - 1] : "";
        t.is_locked = row[i++] ? std::stoi(row[i - 1]) != 0 : false;

        t.db_id = db_id;
        tasks.push_back(std::move(t));
    }

    mysql_free_result(res);
    return tasks;
}

std::vector<Task> fetchTasksFromSQLite(sqlite3* conn, int db_id) {
    std::vector<Task> tasks;

    const char* query = R"(
        SELECT uuid, title, notes, category_id, context_id, 
               project_uuid, topic_id, delegated_to, time_required_minutes, 
               in_focus, due_date, defer_date, created_at, updated_at, 
               is_done, completed_at, link_from, link_to, is_locked 
        FROM Tasks
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(conn, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQLite prepare failed: " << sqlite3_errmsg(conn) << "\n";
        return tasks;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Task t;
        int i = 0;

        auto getText = [&](int col) -> std::string {
            const unsigned char* val = sqlite3_column_text(stmt, col);
            return val ? reinterpret_cast<const char*>(val) : "";
            };

        auto getIntOpt = [&](int col) -> std::optional<int> {
            return sqlite3_column_type(stmt, col) != SQLITE_NULL
                ? std::optional<int>{ sqlite3_column_int(stmt, col) }
            : std::nullopt;
            };

        auto getBool = [&](int col) -> bool {
            return sqlite3_column_type(stmt, col) != SQLITE_NULL
                ? sqlite3_column_int(stmt, col) != 0
                : false;
            };

        auto getStrOpt = [&](int col) -> std::optional<std::string> {
            const unsigned char* val = sqlite3_column_text(stmt, col);
            return val ? std::optional<std::string>{ reinterpret_cast<const char*>(val) } : std::nullopt;
            };

        t.uuid = getText(i++);
        t.title = getText(i++);
        t.notes = getText(i++);

        t.category_id = getIntOpt(i++);
        t.category_label = (t.category_id && categoryLookup.count(*t.category_id))
            ? std::optional<std::string>{ categoryLookup[*t.category_id] } : std::nullopt;

        t.context_id = getIntOpt(i++);
        t.context_label = (t.context_id && contextLookup.count(*t.context_id))
            ? std::optional<std::string>{ contextLookup[*t.context_id] } : std::nullopt;

        t.project_uuid = getStrOpt(i++);
        t.project_title = (t.project_uuid && projectLookup.count(*t.project_uuid))
            ? std::optional<std::string>{ projectLookup[*t.project_uuid] } : std::nullopt;

        t.topic_id = getIntOpt(i++);
        t.topic_label = (t.topic_id && topicLookup.count(*t.topic_id))
            ? std::optional<std::string>{ topicLookup[*t.topic_id] } : std::nullopt;

        t.delegated_to = getIntOpt(i++);
        t.delegate_name = (t.delegated_to && personLookup.count(*t.delegated_to))
            ? std::optional<std::string>{ personLookup[*t.delegated_to] } : std::nullopt;

        t.time_required_minutes = getIntOpt(i++);
        t.in_focus = getBool(i++);
        t.due_date = getStrOpt(i++);
        t.defer_date = getStrOpt(i++);
        t.created_at = getStrOpt(i++);
        t.updated_at = getStrOpt(i++);
        t.is_done = getBool(i++);
        t.completed_at = getStrOpt(i++);
        t.link_from = getText(i++);
        t.link_to = getText(i++);
        t.is_locked = getBool(i++);

        t.db_id = db_id;
        tasks.push_back(std::move(t));
    }

    sqlite3_finalize(stmt);
    return tasks;
}

std::vector<Task> fetchTasksFromDatabase() {
    std::vector<Task> allTasks;

    for (size_t db_id = 0; db_id < allDatabases.size(); ++db_id) {
        const auto& dbConn = allDatabases[db_id];

        if (dbConn.type == DatabaseType::MYSQL) {
            MYSQL* conn = std::get<MYSQL*>(dbConn.connection);
            std::vector<Task> mysqlTasks = fetchTasksFromMySQL(conn, static_cast<int>(db_id));
            allTasks.insert(allTasks.end(), mysqlTasks.begin(), mysqlTasks.end());
        }
        else if (dbConn.type == DatabaseType::SQLITE) {
            sqlite3* conn = std::get<sqlite3*>(dbConn.connection);
            std::vector<Task> sqliteTasks = fetchTasksFromSQLite(conn, static_cast<int>(db_id));
            allTasks.insert(allTasks.end(), sqliteTasks.begin(), sqliteTasks.end());
        }
    }

    return allTasks;
}



static std::string escapeString(MYSQL* conn, const std::string& input) {
    std::string output;
    output.resize(input.length() * 2 + 1); // worst case
    unsigned long len = mysql_real_escape_string(conn, &output[0], input.c_str(), static_cast<unsigned long>(input.length()));
    output.resize(len);
    return output;
}

static void bindOptStr(sqlite3_stmt* stmt, int index, const std::optional<std::string>& val) {
    if (val.has_value())
        sqlite3_bind_text(stmt, index, val->c_str(), -1, SQLITE_TRANSIENT);
    else
        sqlite3_bind_null(stmt, index);
}

void saveTaskToDatabase(Task& task) {
    DatabaseConnection& conn = allDatabases[task.db_id];

    // Step 1: Set updated_at to current time
    {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
        task.updated_at = ss.str();
    }

    std::cout << " Saving task UUID: " << task.uuid
        << " to DB ID: " << task.db_id
        << " (" << (std::holds_alternative<MYSQL*>(conn.connection) ? "MySQL" : "SQLite")
        << ")" << std::endl;

    if (std::holds_alternative<MYSQL*>(conn.connection)) {
        MYSQL* mysql = std::get<MYSQL*>(conn.connection);

        auto escapeOptStr = [&](const std::optional<std::string>& opt) -> std::string {
            return opt.has_value() ? ("'" + escapeString(mysql, *opt) + "'") : "NULL";
            };

        auto escapeOptInt = [&](const std::optional<int>& opt) -> std::string {
            return opt.has_value() ? std::to_string(*opt) : "NULL";
            };

        std::stringstream query;
        query << "REPLACE INTO Tasks ("
            << "uuid, title, notes, category_id, context_id, "
            << "project_uuid, topic_id, delegated_to, time_required_minutes, "
            << "in_focus, due_date, defer_date, created_at, updated_at, "
            << "is_done, completed_at, link_from, link_to, is_locked"
            << ") VALUES ("
            << "'" << escapeString(mysql, task.uuid) << "', "
            << "'" << escapeString(mysql, task.title) << "', "
            << "'" << escapeString(mysql, task.notes) << "', "
            << escapeOptInt(task.category_id) << ", "
            << escapeOptInt(task.context_id) << ", "
            << escapeOptStr(task.project_uuid) << ", "
            << escapeOptInt(task.topic_id) << ", "
            << escapeOptInt(task.delegated_to) << ", "
            << escapeOptInt(task.time_required_minutes) << ", "
            << (task.in_focus ? "1" : "0") << ", "
            << escapeOptStr(task.due_date) << ", "
            << escapeOptStr(task.defer_date) << ", "
            << escapeOptStr(task.created_at) << ", "
            << escapeOptStr(task.updated_at) << ", "
            << (task.is_done ? "1" : "0") << ", "
            << escapeOptStr(task.completed_at) << ", "
            << escapeOptStr(task.link_from) << ", "
            << escapeOptStr(task.link_to) << ", "
            << (task.is_locked ? "1" : "0")
            << ")";

        std::string queryStr = query.str();
        std::cout << " MySQL full query:\n" << queryStr << std::endl;

        if (mysql_query(mysql, queryStr.c_str()) != 0) {
            std::cerr << " MySQL error in saveTaskToDatabase: " << mysql_error(mysql) << std::endl;
        }
        else {
            std::cout << " MySQL save successful." << std::endl;
        }
    }
    else {
        sqlite3* sqlite = std::get<sqlite3*>(conn.connection);
        sqlite3_stmt* stmt = nullptr;

        const char* sql = R"(
            REPLACE INTO Tasks (
                uuid, title, notes,
                category_id, context_id, project_uuid, topic_id, delegated_to,
                time_required_minutes, in_focus,
                due_date, defer_date, created_at, updated_at,
                is_done, completed_at,
                link_from, link_to,
                is_locked
            ) VALUES (
                ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?
            );
        )";

        std::cout << " Preparing SQLite REPLACE for UUID: " << task.uuid << std::endl;

        if (sqlite3_prepare_v2(sqlite, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << " SQLite prepare error: " << sqlite3_errmsg(sqlite) << std::endl;
            return;
        }

        auto bindStr = [&](int idx, const std::string& value) {
            sqlite3_bind_text(stmt, idx, value.c_str(), -1, SQLITE_TRANSIENT);
            };

        auto bindOptStr = [&](int idx, const std::optional<std::string>& value) {
            if (value.has_value())
                sqlite3_bind_text(stmt, idx, value->c_str(), -1, SQLITE_TRANSIENT);
            else
                sqlite3_bind_null(stmt, idx);
            };

        auto bindOptInt = [&](int idx, const std::optional<int>& value) {
            if (value.has_value())
                sqlite3_bind_int(stmt, idx, *value);
            else
                sqlite3_bind_null(stmt, idx);
            };

        int i = 1;
        bindStr(i++, task.uuid);
        bindStr(i++, task.title);
        bindStr(i++, task.notes);

        bindOptInt(i++, task.category_id);
        bindOptInt(i++, task.context_id);
        bindOptStr(i++, task.project_uuid);
        bindOptInt(i++, task.topic_id);
        bindOptInt(i++, task.delegated_to);

        bindOptInt(i++, task.time_required_minutes);
        sqlite3_bind_int(stmt, i++, task.in_focus ? 1 : 0);

        bindOptStr(i++, task.due_date);
        bindOptStr(i++, task.defer_date);
        bindOptStr(i++, task.created_at);
        bindOptStr(i++, task.updated_at);

        sqlite3_bind_int(stmt, i++, task.is_done ? 1 : 0);
        bindOptStr(i++, task.completed_at);
        bindOptStr(i++, task.link_from);
        bindOptStr(i++, task.link_to);

        sqlite3_bind_int(stmt, i++, task.is_locked ? 1 : 0);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << " SQLite step error: " << sqlite3_errmsg(sqlite) << std::endl;
        }
        else {
            std::cout << " SQLite save successful for UUID: " << task.uuid << std::endl;
        }

        sqlite3_finalize(stmt);
    }
}

void moveTaskToDatabase(Task& task, int old_db_id) {
    if (old_db_id == task.db_id) {
        std::cerr << " Tried to move task to the same database; skipping.\n";
        return;
    }

    DatabaseConnection& oldConn = allDatabases[old_db_id];
    DatabaseConnection& newConn = allDatabases[task.db_id];

    // First, delete from old DB
    if (std::holds_alternative<MYSQL*>(oldConn.connection)) {
        MYSQL* conn = std::get<MYSQL*>(oldConn.connection);
        std::string query = "DELETE FROM Tasks WHERE uuid = '" + task.uuid + "'";
        if (mysql_query(conn, query.c_str()) != 0) {
            std::cerr << " Failed to delete task from MySQL DB : " << mysql_error(conn) << std::endl;
        }
        else {
            std::cout << " Old task deleted from MySQL DB\n";
        }
    }
    else if (std::holds_alternative<sqlite3*>(oldConn.connection)) {
        sqlite3* db = std::get<sqlite3*>(oldConn.connection);
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "DELETE FROM Tasks WHERE uuid = ?";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, task.uuid.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << " Failed to delete task from SQLite DB: " << sqlite3_errmsg(db) << std::endl;
            }
            else {
                std::cout << " Old task deleted from SQLite DB\n";
            }
            sqlite3_finalize(stmt);
        }
        else {
            std::cerr << " Failed to prepare DELETE in SQLite: " << sqlite3_errmsg(db) << std::endl;
        }
    }
    else {
        std::cerr << " Unknown database type when deleting old task.\n";
    }

    // Now insert into the new DB
    saveTaskToDatabase(task);
}