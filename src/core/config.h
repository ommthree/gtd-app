#pragma once
#include <string>

namespace AppConfig {
    // Database paths (can now be deprecated if you're using JSON config)
    //inline const std::string kSharedDatabasePath = "Y:/gtd-app/db/gtd_shared.db";

    // Config file locations
    inline const std::string kDatabaseConfigPath = "Y:/gtd-app/config/database_config.json";
    inline const std::string kTableMappingPath = "Y:/gtd-app/config/table_map.json";
}