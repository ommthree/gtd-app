#ifndef TASK_FILTER_CRITERIA_H
#define TASK_FILTER_CRITERIA_H

#include <optional>
#include <set>
#include <string>

struct TaskFilterCriteria {
    // Optional booleans: if set, filter accordingly
    std::optional<bool> in_focus;
    std::optional<bool> is_done;

    // Category filters
    bool allowAllCategories = true;
    std::set<int> allowed_category_ids;

    // Context filters
    bool allowAllContexts = true;
    std::set<int> allowed_context_ids;

    // Topic filters
    bool allowAllTopics = true;
    std::set<int> allowed_topic_ids;

    // Delegate filters (Person)
    bool allowAllDelegates = true;
    std::set<int> allowed_delegate_ids;

    // Project filters (UUID string)
    bool allowAllProjects = true;
    std::set<std::string> allowed_project_uuids;

    void clear() {
        in_focus.reset();
        is_done.reset();

        allowAllCategories = true;
        allowed_category_ids.clear();

        allowAllContexts = true;
        allowed_context_ids.clear();

        allowAllTopics = true;
        allowed_topic_ids.clear();

        allowAllDelegates = true;
        allowed_delegate_ids.clear();

        allowAllProjects = true;
        allowed_project_uuids.clear();
    }
};

#endif // TASK_FILTER_CRITERIA_H