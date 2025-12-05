#pragma once
#include <SDL2/SDL.h>

// --------------------------------------------------------------
// Mouse click handler for SDL2.
// Tracks mouse button presses, releases, and position.
// --------------------------------------------------------------
class Click {
public:
    // Called once per frame with the SDL_Event from your main loop
    void handleEvent(const SDL_Event& e);

    // Query functions
    bool leftPressed() const { return left_down; }
    bool leftClicked() const { return left_clicked; }
    bool rightPressed() const { return right_down; }
    bool rightClicked() const { return right_clicked; }

    int x() const { return mx; }
    int y() const { return my; }

    // Utility: did click occur inside a rect?
    bool inside(const SDL_Rect& r) const;

private:
    int mx = 0, my = 0;
    bool left_down = false;
    bool left_clicked = false;
    bool right_down = false;
    bool right_clicked = false;
};
