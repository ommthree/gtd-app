// card_view.cpp

#include "card_view.h"
#include "imgui.h"
#include <vector>
#include <string>
#include <algorithm>
#include "task.h"  
#include <map>
#include "lookup_maps.h"




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
    if (ImGui::InputTextMultiline("Notes", buffer, sizeof(buffer))) {
        task_.notes = std::string(buffer);
    }

    ImGui::Checkbox("Done", &task_.is_done);
    ImGui::Checkbox("In Focus", &task_.in_focus);

    // Category dropdown
    {
        std::vector<int> categoryIds;
        std::vector<std::string> categoryLabels;
        for (const auto& pair : categoryNames) {
            categoryIds.push_back(pair.first);
            categoryLabels.push_back(pair.second);
        }

        int selectedIndex = 0;
        if (task_.category_id) {
            auto it = std::find(categoryIds.begin(), categoryIds.end(), task_.category_id.value());
            if (it != categoryIds.end()) {
                selectedIndex = static_cast<int>(std::distance(categoryIds.begin(), it));
            }
        }

        if (ImGui::Combo("Category", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& labels = *static_cast<std::vector<std::string>*>(data);
                if (idx < 0 || idx >= static_cast<int>(labels.size())) return false;
                *out_text = labels[idx].c_str();
                return true;
            }, static_cast<void*>(&categoryLabels), static_cast<int>(categoryLabels.size()))) {
            task_.category_id = categoryIds[selectedIndex];
        }
    }

    // Context dropdown
    {
        std::vector<int> contextIds;
        std::vector<std::string> contextLabels;
        for (const auto& [id, label] : contextNames) {
            contextIds.push_back(id);
            contextLabels.push_back(label);
        }

        int selectedIndex = 0;
        if (task_.context_id) {
            auto it = std::find(contextIds.begin(), contextIds.end(), task_.context_id.value());
            if (it != contextIds.end()) {
                selectedIndex = static_cast<int>(std::distance(contextIds.begin(), it));
            }
        }

        if (ImGui::Combo("Context", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& labels = *static_cast<std::vector<std::string>*>(data);
                if (idx < 0 || idx >= static_cast<int>(labels.size())) return false;
                *out_text = labels[idx].c_str();
                return true;
            }, static_cast<void*>(&contextLabels), static_cast<int>(contextLabels.size()))) {
            task_.context_id = contextIds[selectedIndex];
        }
    }

    // Project dropdown
    {
        std::vector<std::string> projectUUIDs;
        std::vector<std::string> projectTitles;
        for (const auto& [uuid, title] : projectTitlesMap) {
            projectUUIDs.push_back(uuid);
            projectTitles.push_back(title);
        }

        int selectedIndex = 0;
        if (task_.project_uuid) {
            auto it = std::find(projectUUIDs.begin(), projectUUIDs.end(), task_.project_uuid.value());
            if (it != projectUUIDs.end()) {
                selectedIndex = static_cast<int>(std::distance(projectUUIDs.begin(), it));
            }
        }

        if (ImGui::Combo("Project", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& titles = *static_cast<std::vector<std::string>*>(data);
                if (idx < 0 || idx >= static_cast<int>(titles.size())) return false;
                *out_text = titles[idx].c_str();
                return true;
            }, static_cast<void*>(&projectTitles), static_cast<int>(projectTitles.size()))) {
            task_.project_uuid = projectUUIDs[selectedIndex];
        }
    }

    // Topic dropdown
    {
        std::vector<int> topicIds;
        std::vector<std::string> topicLabels;
        for (const auto& [id, label] : topicNames) {
            topicIds.push_back(id);
            topicLabels.push_back(label);
        }

        int selectedIndex = 0;
        if (task_.topic_id) {
            auto it = std::find(topicIds.begin(), topicIds.end(), task_.topic_id.value());
            if (it != topicIds.end()) {
                selectedIndex = static_cast<int>(std::distance(topicIds.begin(), it));
            }
        }

        if (ImGui::Combo("Topic", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& labels = *static_cast<std::vector<std::string>*>(data);
                if (idx < 0 || idx >= static_cast<int>(labels.size())) return false;
                *out_text = labels[idx].c_str();
                return true;
            }, static_cast<void*>(&topicLabels), static_cast<int>(topicLabels.size()))) {
            task_.topic_id = topicIds[selectedIndex];
        }
    }

    // Delegate dropdown
    {
        std::vector<int> delegateIds;
        std::vector<std::string> delegateNames;
        for (const auto& [id, name] : personNames) {
            delegateIds.push_back(id);
            delegateNames.push_back(name);
        }

        int selectedIndex = 0;
        if (task_.delegated_to) {
            auto it = std::find(delegateIds.begin(), delegateIds.end(), task_.delegated_to.value());
            if (it != delegateIds.end()) {
                selectedIndex = static_cast<int>(std::distance(delegateIds.begin(), it));
            }
        }

        if (ImGui::Combo("Delegate", &selectedIndex,
            [](void* data, int idx, const char** out_text) {
                auto& names = *static_cast<std::vector<std::string>*>(data);
                if (idx < 0 || idx >= static_cast<int>(names.size())) return false;
                *out_text = names[idx].c_str();
                return true;
            }, static_cast<void*>(&delegateNames), static_cast<int>(delegateNames.size()))) {
            task_.delegated_to = delegateIds[selectedIndex];
        }
    }

    // Display-only metadata
    ImGui::Separator();
    ImGui::TextDisabled("Locked: %s", task_.is_locked ? "Yes" : "No");
    ImGui::TextDisabled("Created: %s", task_.created_at ? task_.created_at->c_str() : "");
    ImGui::TextDisabled("Defer:   %s", task_.defer_date ? task_.defer_date->c_str() : "");
    ImGui::TextDisabled("Due:     %s", task_.due_date ? task_.due_date->c_str() : "");
    ImGui::TextDisabled("Done at: %s", task_.completed_at ? task_.completed_at->c_str() : "");

    ImGui::Separator();

    if (ImGui::Button("Process")) {
        // Stub for reassigning category
    }

    if (!task_.is_locked && ImGui::Button("Delete")) {
        // Stub for deletion action
    }
}