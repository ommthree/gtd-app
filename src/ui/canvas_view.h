#pragma once

#include "core/task.h"
#include "card_view.h"
#include "task_filter_criteria.h"

#include <vector>
#include <imgui.h>

class CanvasView {
public:
    CanvasView();

    void setTasks(std::vector<Task>& tasks);
    void render();
    void setFilterCriteria(const TaskFilterCriteria& criteria);

private:
    // Data
    std::vector<Task>    allTasks_;   // master list
    std::vector<CardView> cards_;     // filtered views

    // View state
    ImVec2 panOffset_;                // panning offset
    ImVec2 lastMousePos_;             // (reserved for future use)
    float  zoom_;                     // zoom factor
    bool   scaleText_;                // whether to scale fonts with zoom

    // Filters
    TaskFilterCriteria filter_;

    // Helpers
    void applyFilter();
    bool taskMatchesFilter(const Task& t) const;
};