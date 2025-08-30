#pragma once

#include <string>
#include <map>
#include <mysql.h>
#include <sqlite3.h>

// === Global lookup maps ===
// All map from ID to name, except Projects (which maps from UUID string)
extern std::map<std::string, std::string> projectLookup;
extern std::map<int, std::string> contextLookup;
extern std::map<int, std::string> topicLookup;
extern std::map<int, std::string> personLookup;
extern std::map<int, std::string> categoryLookup;

// === Populates all lookup maps from all databases ===
void populateLookupMaps();