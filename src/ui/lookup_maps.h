#ifndef CARD_VIEW_GLOBALS_H
#define CARD_VIEW_GLOBALS_H

#pragma once

#include <map>
#include <string>
#include <mysql.h>  //Important: this ensures MYSQL is known here

// Declare shared lookup tables
extern std::map<int, std::string> categoryNames;
extern std::map<int, std::string> contextNames;
extern std::map<std::string, std::string> projectTitlesMap;
extern std::map<int, std::string> topicNames;
extern std::map<int, std::string> personNames;

//Fix declaration: match the definition
void populateLookupMaps(MYSQL* mysql_conn);

#endif // CARD_VIEW_GLOBALS_H