#include "lookup_maps.h"
#include <iostream>
#include <vector>
#include <mysql.h>

// Define the global variables (allocate memory once)
std::map<int, std::string> categoryNames;
std::map<int, std::string> contextNames;
std::map<std::string, std::string> projectTitlesMap;
std::map<int, std::string> topicNames;
std::map<int, std::string> personNames;

void populateLookupMaps(MYSQL* conn) {
    struct QueryDef {
        const char* sql;
        std::map<int, std::string>& target;
        const char* name;
    };

    std::vector<QueryDef> queries = {
        {"SELECT id, name FROM Categories", categoryNames, "Categories"},
        {"SELECT id, name FROM Contexts", contextNames, "Contexts"},
        {"SELECT id, name FROM Topics", topicNames, "Topics"},
        {"SELECT id, name FROM People", personNames, "People"},
    };

    for (const auto& q : queries) {
        if (mysql_query(conn, q.sql) == 0) {
            MYSQL_RES* res = mysql_store_result(conn);
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                if (row[0] && row[1]) {
                    q.target[std::stoi(row[0])] = row[1];
                }
            }
            mysql_free_result(res);
        }
        else {
            std::cerr << "Query failed for table " << q.name << ": " << mysql_error(conn) << std::endl;
        }
    }

    // Project titles: key is UUID (string)
    if (mysql_query(conn, "SELECT uuid, name FROM Projects") == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res))) {
            if (row[0] && row[1]) {
                projectTitlesMap[row[0]] = row[1];
            }
        }
        mysql_free_result(res);
    }
    else {
        std::cerr << "Query failed for Projects: " << mysql_error(conn) << std::endl;
    }

    std::cout << "Lookup maps populated:\n"
        << "  Categories: " << categoryNames.size() << "\n"
        << "  Contexts:   " << contextNames.size() << "\n"
        << "  Topics:     " << topicNames.size() << "\n"
        << "  People:     " << personNames.size() << "\n"
        << "  Projects:   " << projectTitlesMap.size() << std::endl;
}