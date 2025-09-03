// card_view.h

#ifndef CARD_VIEW_H
#define CARD_VIEW_H

#include <string>
#include "core/task.h"

class CardView {
public:
    explicit CardView(Task& task);

    // Draws the card, scaling size based on zoom factor
    void draw(float zoom = 1.0f);

private:
    void drawFront(float zoom);
    void drawBack(float zoom);

    Task& task_;
    bool is_flipped_ = false;
};

#endif // CARD_VIEW_H