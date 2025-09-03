#include "card_view.h"
#include "imgui.h"
#include <vector>
#include <string>
#include <algorithm>
#include "task.h"  
#include <map>
#include "core/lookup_maps.h"
#include "core/database_registry.h"
#include "core/database.h"

CardView::CardView(Task& task)
    : task_(task)
{
}

void CardView::draw(float zoom) {
    const float baseWidth = 300.0f;
    const float baseHeight = 200.0f;
    const float width = baseWidth * zoom;
    const float height = baseHeight * zoom;

    ImGui::BeginChild(task_.uuid.c_str(), ImVec2(width, height), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    if (ImGui::Button("Flip")) {
        is_flipped_ = !is_flipped_;
    }

    ImGui::Separator();

    if (is_flipped_) {
        drawBack(zoom);
    }
    else {
        drawFront(zoom);
    }

    ImGui::EndChild();
}

void CardView::drawFront(float zoom) {
    ImGui::TextWrapped("%s", task_.title.c_str());

    if (task_.in_focus) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "FOCUS");
    }
    if (task_.is_done) {
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "DONE");
    }
}

void CardView::drawBack(float zoom) {
    static char buffer[1024];
    strncpy(buffer, task_.notes.c_str(), sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    bool changed = false;
    bool dbChanged = false;
    int originalDbId = task_.db_id;
    int oldDbId = task_.db_id;

    if (ImGui::InputTextMultiline("Notes", buffer, sizeof(buffer))) {
        task_.notes = std::string(buffer);
        changed = true;
    }

    if (ImGui::Checkbox("Done", &task_.is_done)) changed = true;
    if (ImGui::Checkbox("In Focus", &task_.in_focus)) changed = true;

    // === Category dropdown ===
    {
        std::vector<int> ids;
        std::vector<std::string> labels;
        for (const auto& [id, label] : categoryLookup) {
            ids.push_back(id);
            labels.push_back(label);
        }

        int selectedIndex = 0;
        if (task_.category_id) {
            auto it = std::find(ids.begin(), ids.end(), *task_.category_id);
            if (it != ids.end()) selectedIndex = static_cast<int>(it - ids.begin());
        }

        if (ImGui::Combo("Category", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& v = *static_cast<std::vector<std::string>*>(data);
                *out_text = v[idx].c_str(); return true;
            }, static_cast<void*>(&labels), static_cast<int>(labels.size()))) {
            task_.category_id = ids[selectedIndex];
            changed = true;
        }
    }

    // === Context dropdown ===
    {
        std::vector<int> ids;
        std::vector<std::string> labels;
        for (const auto& [id, label] : contextLookup) {
            ids.push_back(id);
            labels.push_back(label);
        }

        int selectedIndex = 0;
        if (task_.context_id) {
            auto it = std::find(ids.begin(), ids.end(), *task_.context_id);
            if (it != ids.end()) selectedIndex = static_cast<int>(it - ids.begin());
        }

        if (ImGui::Combo("Context", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& v = *static_cast<std::vector<std::string>*>(data);
                *out_text = v[idx].c_str(); return true;
            }, static_cast<void*>(&labels), static_cast<int>(labels.size()))) {
            task_.context_id = ids[selectedIndex];
            changed = true;
        }
    }

    // === Project dropdown ===
    {
        std::vector<std::string> uuids;
        std::vector<std::string> titles;
        for (const auto& [uuid, title] : projectLookup) {
            uuids.push_back(uuid);
            titles.push_back(title);
        }

        int selectedIndex = 0;
        if (task_.project_uuid) {
            auto it = std::find(uuids.begin(), uuids.end(), *task_.project_uuid);
            if (it != uuids.end()) selectedIndex = static_cast<int>(it - uuids.begin());
        }

        if (ImGui::Combo("Project", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& v = *static_cast<std::vector<std::string>*>(data);
                *out_text = v[idx].c_str(); return true;
            }, static_cast<void*>(&titles), static_cast<int>(titles.size()))) {
            task_.project_uuid = uuids[selectedIndex];
            changed = true;
        }
    }

    // === Topic dropdown ===
    {
        std::vector<int> ids;
        std::vector<std::string> labels;
        for (const auto& [id, label] : topicLookup) {
            ids.push_back(id);
            labels.push_back(label);
        }

        int selectedIndex = 0;
        if (task_.topic_id) {
            auto it = std::find(ids.begin(), ids.end(), *task_.topic_id);
            if (it != ids.end()) selectedIndex = static_cast<int>(it - ids.begin());
        }

        if (ImGui::Combo("Topic", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& v = *static_cast<std::vector<std::string>*>(data);
                *out_text = v[idx].c_str(); return true;
            }, static_cast<void*>(&labels), static_cast<int>(labels.size()))) {
            task_.topic_id = ids[selectedIndex];
            changed = true;
        }
    }

    // === Delegate dropdown ===
    {
        std::vector<int> ids;
        std::vector<std::string> names;
        for (const auto& [id, name] : personLookup) {
            ids.push_back(id);
            names.push_back(name);
        }

        int selectedIndex = 0;
        if (task_.delegated_to) {
            auto it = std::find(ids.begin(), ids.end(), *task_.delegated_to);
            if (it != ids.end()) selectedIndex = static_cast<int>(it - ids.begin());
        }

        if (ImGui::Combo("Delegate", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& v = *static_cast<std::vector<std::string>*>(data);
                *out_text = v[idx].c_str(); return true;
            }, static_cast<void*>(&names), static_cast<int>(names.size()))) {
            task_.delegated_to = ids[selectedIndex];
            changed = true;
        }
    }

    // === Database dropdown ===
    {
        int selectedDb = task_.db_id;
        if (ImGui::Combo("Database", &selectedDb,
            [](void* data, int idx, const char** out_text) {
                auto& v = *static_cast<std::vector<std::string>*>(data);
                *out_text = v[idx].c_str(); return true;
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