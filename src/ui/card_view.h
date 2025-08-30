// card_view.h

#ifndef CARD_VIEW_H
#define CARD_VIEW_H

#include <string>

#include "core/task.h"

class CardView {
public:
    explicit CardView(Task& task);

    void draw();  // Draws either front or back depending on flip state

private:
    void drawFront();
    void drawBack();

    Task& task_;
    bool is_flipped_ = false;
};

#endif // CARD_VIEW_H