#include "card_view.h"
#include "imgui.h"
#include <vector>
#include <string>
#include <algorithm>
#include "task.h"  
#include <map>
#include "core/lookup_maps.h"
#include "core/database_registry.h" // for databaseNames
#include "core/database.h" // for saveTaskToDatabase, moveTaskToDatabase

CardView::CardView(Task& task)
    : task_(task)
{
}

void CardView::draw() {
    ImGui::BeginChild(task_.uuid.c_str(), ImVec2(300, 200), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    if (ImGui::Button("Flip")) {
        is_flipped_ = !is_flipped_;
    }

    ImGui::Separator();

    if (is_flipped_) {
        drawBack();
    }
    else {
        drawFront();
    }

    ImGui::EndChild();
}

void CardView::drawFront() {
    ImGui::TextWrapped("%s", task_.title.c_str());
    if (task_.in_focus) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "FOCUS");
    }
    if (task_.is_done) {
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "DONE");
    }
}

void CardView::drawBack() {
    static char buffer[1024];
    strncpy(buffer, task_.notes.c_str(), sizeof(buffer));

    bool changed = false;
    bool dbChanged = false;
    int originalDbId = task_.db_id;
    int oldDbId = task_.db_id;

    if (ImGui::InputTextMultiline("Notes", buffer, sizeof(buffer))) {
        task_.notes = std::string(buffer);
        changed = true;
    }

    if (ImGui::Checkbox("Done", &task_.is_done)) {
        changed = true;
    }

    if (ImGui::Checkbox("In Focus", &task_.in_focus)) {
        changed = true;
    }

    // === Category dropdown ===
    {
        std::vector<int> categoryIds;
        std::vector<std::string> categoryLabels;
        for (const auto& [id, label] : categoryLookup) {
            categoryIds.push_back(id);
            categoryLabels.push_back(label);
        }

        int selectedIndex = 0;
        if (task_.category_id) {
            auto it = std::find(categoryIds.begin(), categoryIds.end(), *task_.category_id);
            if (it != categoryIds.end()) {
                selectedIndex = static_cast<int>(it - categoryIds.begin());
            }
        }

        if (ImGui::Combo("Category", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& labels = *static_cast<std::vector<std::string>*>(data);
                if (idx >= 0 && idx < static_cast<int>(labels.size())) {
                    *out_text = labels[idx].c_str();
                    return true;
                }
                return false;
            }, static_cast<void*>(&categoryLabels), static_cast<int>(categoryLabels.size()))) {
            task_.category_id = categoryIds[selectedIndex];
            changed = true;
        }
    }

    // === Context dropdown ===
    {
        std::vector<int> contextIds;
        std::vector<std::string> contextLabels;
        for (const auto& [id, label] : contextLookup) {
            contextIds.push_back(id);
            contextLabels.push_back(label);
        }

        int selectedIndex = 0;
        if (task_.context_id) {
            auto it = std::find(contextIds.begin(), contextIds.end(), *task_.context_id);
            if (it != contextIds.end()) {
                selectedIndex = static_cast<int>(it - contextIds.begin());
            }
        }

        if (ImGui::Combo("Context", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& labels = *static_cast<std::vector<std::string>*>(data);
                if (idx >= 0 && idx < static_cast<int>(labels.size())) {
                    *out_text = labels[idx].c_str();
                    return true;
                }
                return false;
            }, static_cast<void*>(&contextLabels), static_cast<int>(contextLabels.size()))) {
            task_.context_id = contextIds[selectedIndex];
            changed = true;
        }
    }

    // === Project dropdown ===
    {
        std::vector<std::string> projectUUIDs;
        std::vector<std::string> projectTitles;
        for (const auto& [uuid, title] : projectLookup) {
            projectUUIDs.push_back(uuid);
            projectTitles.push_back(title);
        }

        int selectedIndex = 0;
        if (task_.project_uuid) {
            auto it = std::find(projectUUIDs.begin(), projectUUIDs.end(), *task_.project_uuid);
            if (it != projectUUIDs.end()) {
                selectedIndex = static_cast<int>(it - projectUUIDs.begin());
            }
        }

        if (ImGui::Combo("Project", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& titles = *static_cast<std::vector<std::string>*>(data);
                if (idx >= 0 && idx < static_cast<int>(titles.size())) {
                    *out_text = titles[idx].c_str();
                    return true;
                }
                return false;
            }, static_cast<void*>(&projectTitles), static_cast<int>(projectTitles.size()))) {
            task_.project_uuid = projectUUIDs[selectedIndex];
            changed = true;
        }
    }

    // === Topic dropdown ===
    {
        std::vector<int> topicIds;
        std::vector<std::string> topicLabels;
        for (const auto& [id, label] : topicLookup) {
            topicIds.push_back(id);
            topicLabels.push_back(label);
        }

        int selectedIndex = 0;
        if (task_.topic_id) {
            auto it = std::find(topicIds.begin(), topicIds.end(), *task_.topic_id);
            if (it != topicIds.end()) {
                selectedIndex = static_cast<int>(it - topicIds.begin());
            }
        }

        if (ImGui::Combo("Topic", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& labels = *static_cast<std::vector<std::string>*>(data);
                if (idx >= 0 && idx < static_cast<int>(labels.size())) {
                    *out_text = labels[idx].c_str();
                    return true;
                }
                return false;
            }, static_cast<void*>(&topicLabels), static_cast<int>(topicLabels.size()))) {
            task_.topic_id = topicIds[selectedIndex];
            changed = true;
        }
    }

    // === Delegate dropdown ===
    {
        std::vector<int> delegateIds;
        std::vector<std::string> delegateNames;
        for (const auto& [id, name] : personLookup) {
            delegateIds.push_back(id);
            delegateNames.push_back(name);
        }

        int selectedIndex = 0;
        if (task_.delegated_to) {
            auto it = std::find(delegateIds.begin(), delegateIds.end(), *task_.delegated_to);
            if (it != delegateIds.end()) {
                selectedIndex = static_cast<int>(it - delegateIds.begin());
            }
        }

        if (ImGui::Combo("Delegate", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& names = *static_cast<std::vector<std::string>*>(data);
                if (idx >= 0 && idx < static_cast<int>(names.size())) {
                    *out_text = names[idx].c_str();
                    return true;
                }
                return false;
            }, static_cast<void*>(&delegateNames), static_cast<int>(delegateNames.size()))) {
            task_.delegated_to = delegateIds[selectedIndex];
            changed = true;
        }
    }

    // === Database dropdown ===
    {
        int selectedDb = task_.db_id;
        if (ImGui::Combo("Database", &selectedDb,
            [](void* data, int idx, const char** out_text) {
                const auto& names = *static_cast<std::vector<std::string>*>(data);
                if (idx >= 0 && idx < static_cast<int>(names.size())) {
                    *out_text = names[idx].c_str();
                    return true;
                }
                return false;
            }, static_cast<void*>(&databaseNames), static_cast<int>(databaseNames.size()))) {
            if (selectedDb != task_.db_id) {
                task_.db_id = selectedDb;
                dbChanged = true;
            }
        }
    }

    ImGui::Separator();
    ImGui::TextDisabled("Locked: %s", task_.is_locked ? "Yes" : "No");
    ImGui::TextDisabled("Created: %s", task_.created_at ? task_.created_at->c_str() : "");
    ImGui::TextDisabled("Defer:   %s", task_.defer_date ? task_.defer_date->c_str() : "");
    ImGui::TextDisabled("Due:     %s", task_.due_date ? task_.due_date->c_str() : "");
    ImGui::TextDisabled("Done at: %s", task_.completed_at ? task_.completed_at->c_str() : "");

    ImGui::Separator();
    if (ImGui::Button("Process")) {
        // TODO
    }

    if (!task_.is_locked && ImGui::Button("Delete")) {
        // TODO
    }

    if (changed || dbChanged) {
        if (dbChanged && task_.db_id != originalDbId) {
            moveTaskToDatabase(task_, oldDbId);
        }
        else {
            saveTaskToDatabase(task_);
        }
    }
}