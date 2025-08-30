#include "database.h"
#include "sqlite3.h"
#include <iostream>
#include "task.h"
#include <mysql.h>
#include <vector>
#include <optional>
#include <map>
#include <variant>
#include "core/database_registry.h"

std::vector<Task> fetchTasksFromMySQL(MYSQL* conn, int db_id) {
    std::vector<Task> tasks;

    // === Load lookup tables ===
    std::map<int, std::string> categoryLabels;
    std::map<int, std::string> contextLabels;
    std::map<std::string, std::string> projectTitles;
    std::map<int, std::string> topicLabels;
    std::map<int, std::string> delegateNames;

    auto fetchMap = [&](const char* sql, auto& map, auto convertKey) {
        if (mysql_query(conn, sql) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            if (res) {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res))) {
                    if (row[0] && row[1]) {
                        map[convertKey(row[0])] = row[1];
                    }
                }
                mysql_free_result(res);
            }
        }
        };

    fetchMap("SELECT id, label FROM Categories", categoryLabels, [](const char* s) { return std::stoi(s); });
    fetchMap("SELECT id, label FROM Contexts", contextLabels, [](const char* s) { return std::stoi(s); });
    fetchMap("SELECT uuid, title FROM Projects", projectTitles, [](const char* s) { return std::string(s); });
    fetchMap("SELECT id, label FROM Topics", topicLabels, [](const char* s) { return std::stoi(s); });
    fetchMap("SELECT id, name FROM People", delegateNames, [](const char* s) { return std::stoi(s); });

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
        t.category_label = (t.category_id && categoryLabels.count(*t.category_id))
            ? std::optional<std::string>{ categoryLabels[*t.category_id] } : std::nullopt;
        ++i;

        t.context_id = row[i] ? std::optional<int>{ std::stoi(row[i]) } : std::nullopt;
        t.context_label = (t.context_id && contextLabels.count(*t.context_id))
            ? std::optional<std::string>{ contextLabels[*t.context_id] } : std::nullopt;
        ++i;

        t.project_uuid = row[i] ? std::optional<std::string>{ row[i] } : std::nullopt;
        t.project_title = (t.project_uuid && projectTitles.count(*t.project_uuid))
            ? std::optional<std::string>{ projectTitles[*t.project_uuid] } : std::nullopt;
        ++i;

        t.topic_id = row[i] ? std::optional<int>{ std::stoi(row[i]) } : std::nullopt;
        t.topic_label = (t.topic_id && topicLabels.count(*t.topic_id))
            ? std::optional<std::string>{ topicLabels[*t.topic_id] } : std::nullopt;
        ++i;

        t.delegated_to = row[i] ? std::optional<int>{ std::stoi(row[i]) } : std::nullopt;
        t.delegate_name = (t.delegated_to && delegateNames.count(*t.delegated_to))
            ? std::optional<std::string>{ delegateNames[*t.delegated_to] } : std::nullopt;
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
    // TODO: Implement SQLite version of the query
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

void saveTaskToDatabase(const Task& task) {
    if (task.db_id < 0 || task.db_id >= static_cast<int>(allDatabases.size())) {
        std::cerr << "Invalid db_id in task: " << task.db_id << std::endl;
        return;
    }

    const auto& dbConn = allDatabases[task.db_id];

    if (dbConn.type == DatabaseType::MYSQL) {
        MYSQL* conn = std::get<MYSQL*>(dbConn.connection);
        std::string query =
            "REPLACE INTO Tasks (uuid, title, notes, in_focus, is_done, updated_at) VALUES ('" +
            task.uuid + "', '" +
            task.title + "', '" +
            task.notes + "', " +
            (task.in_focus ? "1" : "0") + ", " +
            (task.is_done ? "1" : "0") + ", NOW())";

        if (mysql_query(conn, query.c_str()) != 0) {
            std::cerr << "MySQL save failed: " << mysql_error(conn) << std::endl;
        }

    }
    else if (dbConn.type == DatabaseType::SQLITE) {
        sqlite3* db = std::get<sqlite3*>(dbConn.connection);
        std::string sql =
            "REPLACE INTO Tasks (uuid, title, notes, in_focus, is_done, updated_at) VALUES (?, ?, ?, ?, ?, CURRENT_TIMESTAMP)";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, task.uuid.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, task.title.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, task.notes.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 4, task.in_focus ? 1 : 0);
            sqlite3_bind_int(stmt, 5, task.is_done ? 1 : 0);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
        else {
            std::cerr << "SQLite save failed: " << sqlite3_errmsg(db) << std::endl;
        }
    }
}