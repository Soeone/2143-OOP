#pragma once
#include "CellularAutomaton.hpp"
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

// --------------------------------------------------------------
// Base Class: Screen
// --------------------------------------------------------------
// Purpose:
//   This class defines a generic rendering interface for anything
//   that displays a 2D cellular automaton (or any grid of ints).
//
// Why this is useful:
//   - Allows multiple display backends (text, SDL2, ncurses, GUI)
//   - Code that runs the automaton does NOT depend on how it is drawn
//   - Encourages clean separation of "model" vs "view"
//
// The grid is represented as a vector of vector<int>, where
// each element is the state of a cell (0, 1, or other integers).
// --------------------------------------------------------------
class Screen {
   public:
    // ----------------------------------------------------------
    // render():
    //   Pure virtual function that must be implemented by any
    //   subclass. Responsible for drawing the given 2D grid.
    //
    //   The 'const' means the method does not modify the Screen.
    // ----------------------------------------------------------
    virtual void render(const std::vector<std::vector<int>>& grid) const = 0;

    // ----------------------------------------------------------
    // pause():
    //   Pure virtual. Allows a screen to control timing, frame rate,
    //   or animation pacing by sleeping for 'ms' milliseconds.
    // ----------------------------------------------------------
    virtual void pause(int ms) const = 0;

    // ----------------------------------------------------------
    // Virtual destructor:
    //   Ensures safe cleanup when deleting derived classes through
    //   a base pointer — required for polymorphic classes.
    // ----------------------------------------------------------
    virtual ~Screen() = default;
};

// --------------------------------------------------------------
// TextScreen:
// --------------------------------------------------------------
// A concrete implementation of Screen that renders a grid directly
// to the terminal using ASCII or Unicode characters.
//
// Useful for:
//   - quick demos
//   - debugging visual behavior quietly
//   - environments where no graphics library is available
//
// Implementations of render() and pause() live in TextScreen.cpp
// --------------------------------------------------------------

// --------------------------------------------------------------
// SdlScreen:
// --------------------------------------------------------------
// Renders the automaton using SDL2 in a graphical window
// --------------------------------------------------------------
class SdlScreen : public Screen {
   private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    int cellSize;
    int windowWidth;
    int windowHeight;

   public:
    // Constructor: creates SDL window and renderer
    SdlScreen(int width, int height, int cellSz = 10) 
        : cellSize(cellSz), windowWidth(width), windowHeight(height) 
    {
        // Initialize SDL2
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::runtime_error("SDL initialization failed");
        }

        window = SDL_CreateWindow(
            "Conway's Game of Life",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width,
            height,
            SDL_WINDOW_SHOWN
        );


        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    }

    // Render the grid
    void render(const std::vector<std::vector<int>>& grid) const override {
        // Clear screen (black background)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw live cells (white)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        
        for (size_t row = 0; row < grid.size(); ++row) {
            for (size_t col = 0; col < grid[row].size(); ++col) {
                if (grid[row][col] == 1) {  // Live cell
                    SDL_Rect rect;
                    rect.x = col * cellSize;
                    rect.y = row * cellSize;
                    rect.w = cellSize;
                    rect.h = cellSize;
                   if (grid[row][col] == 1) 
                    {
                     SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White for alive
                    } 
                    else 
                    {
                     SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);     // Dark gray for dead
                    }
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }

        SDL_RenderPresent(renderer);
    }

    void pause(int ms) const override {
        SDL_Delay(ms);
        
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            }
        }
    }

    ~SdlScreen() override {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};