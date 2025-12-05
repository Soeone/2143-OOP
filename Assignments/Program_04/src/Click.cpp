#include <SDL2/SDL.h>
#include "../includes/Click.hpp"

void Click::handleEvent(const SDL_Event& e) {
    left_clicked = false;
    right_clicked = false;

    switch (e.type) {
        case SDL_MOUSEMOTION:
            mx = e.motion.x;
            my = e.motion.y;
            break;

        case SDL_MOUSEBUTTONDOWN:
            mx = e.button.x;
            my = e.button.y;
            if (e.button.button == SDL_BUTTON_LEFT)
                left_down = true;
            if (e.button.button == SDL_BUTTON_RIGHT)
                right_down = true;
            break;

        case SDL_MOUSEBUTTONUP:
            mx = e.button.x;
            my = e.button.y;
            if (e.button.button == SDL_BUTTON_LEFT) {
                left_down = false;
                left_clicked = true;      // registers a "click"
            }
            if (e.button.button == SDL_BUTTON_RIGHT) {
                right_down = false;
                right_clicked = true;
            }
            break;
    }
}

bool Click::inside(const SDL_Rect& r) const {
    return (mx >= r.x && mx <= r.x + r.w &&
            my >= r.y && my <= r.y + r.h);
}