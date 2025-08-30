#ifndef TASK_H
#define TASK_H

#include <string>
#include <optional>
#include <chrono>

struct Task {
    std::string uuid;
    std::string title;
    std::string notes;

    std::optional<int> category_id;
    std::optional<int> context_id;
    std::optional<std::string> project_uuid;
    std::optional<int> topic_id;
    std::optional<int> delegated_to;
    int db_id = 0;  // 0 = shared (MySQL), 1 = local (SQLite), etc.

    std::optional<int> time_required_minutes;

    bool in_focus = false;
    bool is_done = false;
    bool is_locked = false;

    std::optional<std::string> due_date;    // ISO 8601 format
    std::optional<std::string> defer_date;
    std::optional<std::string> created_at;
    std::optional<std::string> updated_at;
    std::optional<std::string> completed_at;

    std::optional<std::string> link_from;
    std::optional<std::string> link_to;

    // ?? Enriched display labels (optional, populated later via JOINs)
    std::optional<std::string> category_label;
    std::optional<std::string> context_label;
    std::optional<std::string> project_title;
    std::optional<std::string> topic_label;
    std::optional<std::string> delegate_name;
};

#endif // TASK_H