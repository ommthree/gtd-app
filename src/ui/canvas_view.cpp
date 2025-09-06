#include "canvas_view.h"
#include <imgui.h>
#include <algorithm> // std::clamp, std::max

CanvasView::CanvasView()
    : panOffset_(0, 0)
    , lastMousePos_(0, 0)
    , zoom_(1.0f)
    , scaleText_(false)
{
    // Default: no constraints (show all)
    filter_.in_focus.reset();
    filter_.is_done.reset();
}

void CanvasView::setTasks(std::vector<Task>& tasks) {
    // Keep our own storage so CardView(Task&) stays valid
    allTasks_ = tasks;
    applyFilter();
}

void CanvasView::setFilterCriteria(const TaskFilterCriteria& criteria) {
    filter_ = criteria;
    applyFilter();
}

bool CanvasView::taskMatchesFilter(const Task& t) const {
    if (filter_.is_done.has_value() && t.is_done != filter_.is_done.value())   return false;
    if (filter_.in_focus.has_value() && t.in_focus != filter_.in_focus.value())  return false;
    // (extend with category/context/topic/delegate/project later)
    return true;
}

void CanvasView::applyFilter() {
    cards_.clear();
    cards_.reserve(allTasks_.size());
    for (Task& t : allTasks_) {
        if (taskMatchesFilter(t)) {
            cards_.emplace_back(t); // CardView(Task&)
        }
    }
}

void CanvasView::render() {
    ImGuiIO& io = ImGui::GetIO();

    // Apply current zoom/font for this frame (affects card layout)
    zoom_ = std::clamp(zoom_, 0.5f, 3.0f);
    io.FontGlobalScale = scaleText_ ? zoom_ : 1.0f;

    // === Canvas content (pannable area) ===
    ImGui::BeginChild("CanvasRegion", ImVec2(0, 0), false,
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

    ImVec2 origin = ImGui::GetCursorScreenPos();

    // Right-button drag to pan the board
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
        ImVec2 delta = io.MouseDelta;
        panOffset_.x += delta.x;
        panOffset_.y += delta.y;
    }

    // Layout
    const float baseCardWidth = 300.0f;
    const float baseCardHeight = 200.0f;
    const float spacing = 20.0f * zoom_;
    const float cardWidth = baseCardWidth * zoom_;
    const float cardHeight = baseCardHeight * zoom_;

    int cardsPerRow = std::max(1, int((ImGui::GetContentRegionAvail().x + spacing) / (cardWidth + spacing)));

    for (size_t i = 0; i < cards_.size(); ++i) {
        int row = int(i / cardsPerRow);
        int col = int(i % cardsPerRow);

        ImVec2 pos(
            origin.x + panOffset_.x + col * (cardWidth + spacing),
            origin.y + panOffset_.y + row * (cardHeight + spacing)
        );

        ImGui::SetCursorScreenPos(pos);
        cards_[i].draw(zoom_);
    }

    ImGui::EndChild();

    // === Floating Controls Overlay (always on top; not panned) ===
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);    // relative to parent window
    ImGui::SetNextWindowBgAlpha(0.9f);

    ImGuiWindowFlags ctrlFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    bool uiChanged = false;
    if (ImGui::Begin("Canvas Controls", nullptr, ctrlFlags)) {
        uiChanged |= ImGui::SliderFloat("Zoom", &zoom_, 0.5f, 3.0f, "%.1fx");
        uiChanged |= ImGui::Checkbox("Scale Text", &scaleText_);

        ImGui::Separator();
        ImGui::Text("Filter");

        // Done tri-state: 0=Any, 1=Done, 2=Not done
        int doneState = 0;
        if (filter_.is_done.has_value()) doneState = filter_.is_done.value() ? 1 : 2;
        const char* doneItems[] = { "Any", "Done", "Not done" };
        if (ImGui::Combo("Done", &doneState, doneItems, 3)) {
            if (doneState == 0) filter_.is_done.reset();
            else                filter_.is_done = (doneState == 1);
            uiChanged = true;
        }

        // Focus tri-state: 0=Any, 1=In Focus, 2=Not in Focus
        int focusState = 0;
        if (filter_.in_focus.has_value()) focusState = filter_.in_focus.value() ? 1 : 2;
        const char* focusItems[] = { "Any", "In Focus", "Not in Focus" };
        if (ImGui::Combo("Focus", &focusState, focusItems, 3)) {
            if (focusState == 0) filter_.in_focus.reset();
            else                 filter_.in_focus = (focusState == 1);
            uiChanged = true;
        }
    }
    ImGui::End();

    if (uiChanged) {
        applyFilter(); // reflect changes next frame
    }
}