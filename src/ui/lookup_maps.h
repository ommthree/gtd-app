#pragma once

#include <string>
#include <map>
#include <mysql.h>
#include <sqlite3.h>

// === Global lookup maps ===
// All map from ID to name, except projectLookup which maps from UUID string to name
extern std::map<int, std::string> categoryLookup;
extern std::map<int, std::string> contextLookup;
extern std::map<int, std::string> topicLookup;
extern std::map<int, std::string> personLookup;
extern std::map<std::string, std::string> projectLookup;

// === Populates all lookup maps from MySQL and SQLite based on table mapping ===
void populateLookupMaps();