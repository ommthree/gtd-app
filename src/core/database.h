#pragma once
#include <string>
#include <vector>
#include "task.h"
#include <mysql.h>
#include <sqlite3.h>


std::vector<Task> fetchTasksFromDatabase();
std::vector<Task> fetchTasksFromMySQL(MYSQL* conn, int db_id);
std::vector<Task> fetchTasksFromSQLite(sqlite3* conn, int db_id);

void saveTaskToDatabase(Task& task);
void moveTaskToDatabase(Task& task, int old_db_id);


class Database {
public:
    bool open(const std::string& path);
    std::vector<Task> getAllTasks();

private:
    void close();
    void reportError(const char* msg);
    void checkAndInitSchema();


    struct sqlite3* db = nullptr;
};