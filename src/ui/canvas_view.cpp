#include "canvas_view.h"
#include "card_view.h"

#include <imgui.h>
#include <algorithm>

CanvasView::CanvasView()
    : zoom_(1.0f), panOffset_(0, 0), lastMousePos_(0, 0), scaleText_(false) {
}

void CanvasView::setTasks(std::vector<Task>& tasks) {
    cards_.clear();
    for (Task& task : tasks) {
        cards_.emplace_back(task);
    }
}

void CanvasView::render() {
    ImGui::BeginChild("CanvasRegion", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();

    ImGuiIO& io = ImGui::GetIO();

    // Draw zoom controls (top-left)
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::Begin("ZoomControl", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("Zoom: %.2fx", zoom_);
    ImGui::SliderFloat("##ZoomSlider", &zoom_, 0.5f, 3.0f, "%.1fx");
    ImGui::Checkbox("Scale Text", &scaleText_);
    ImGui::End();

    // Clamp zoom to avoid card overlap
    zoom_ = std::clamp(zoom_, 0.5f, 3.0f);

    // Adjust font scale globally if enabled
    io.FontGlobalScale = scaleText_ ? zoom_ : 1.0f;

    // Handle panning with right mouse drag
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
        ImVec2 mouseDelta = io.MouseDelta;
        panOffset_.x += mouseDelta.x;
        panOffset_.y += mouseDelta.y;
    }

    // Layout parameters
    const float baseCardWidth = 300.0f;
    const float baseCardHeight = 200.0f;
    const float spacing = 20.0f * zoom_;  // Dynamic spacing

    const float cardWidth = baseCardWidth * zoom_;
    const float cardHeight = baseCardHeight * zoom_;

    int cardsPerRow = std::max(1, int((ImGui::GetContentRegionAvail().x + spacing) / (cardWidth + spacing)));

    for (size_t i = 0; i < cards_.size(); ++i) {
        int row = int(i / cardsPerRow);
        int col = int(i % cardsPerRow);

        ImVec2 pos = ImVec2(
            origin.x + panOffset_.x + col * (cardWidth + spacing),
            origin.y + panOffset_.y + row * (cardHeight + spacing)
        );

        ImGui::SetCursorScreenPos(pos);
        cards_[i].draw(zoom_);
    }

    ImGui::EndChild();
}