#pragma once

#include "core/task.h"
#include "card_view.h"
#include <vector>
#include <imgui.h>

class CanvasView {
public:
    CanvasView();

    void setTasks(std::vector<Task>& tasks);
    void render();

private:
    std::vector<CardView> cards_;
    ImVec2 panOffset_;         // For panning
    ImVec2 lastMousePos_;      // For tracking drag movement
    float zoom_;               // For zoom level
    bool scaleText_ = false; 
};