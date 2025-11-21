/**
 * ============================================================
 *  SDL2 Grid Example
 * ============================================================
 *  This program creates a simple window using SDL2 and
 *  renders a visible grid based on a given cell size.
 *
 *  Concepts introduced:
 *   - Converting grid coordinates to pixel coordinates
 *   - Drawing vertical & horizontal grid lines
 *   - Handling the basic SDL event loop
 *   - Using variables to control cell size, grid width, and height
 *
 */
#include <SDL.h>
#include <cstdlib>
#include <climits>
#include <ctime>
#include <cstddef>
#include <iomanip>
#include <vector>
#include <string>
#include "./includes/argsToJson.hpp"
#include "./includes/json.hpp"
#include <fstream>
#include <iostream>  // For error logging to std::cerr
using json = nlohmann::json;

using namespace std;

int main(int argc, char* argv[]) {
    json params = ArgsToJson(argc, argv);
    cout << params.dump(4) << endl;
    
    struct Cell {
        int x;
        int y;
    };

    // A shape that consists of a name, dimensions, and a list of live cells
    struct Shape {
        string name;
        int width;
        int height;
        vector<Cell> cells;
    };

    ifstream file("./includes/shapes.json");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open shapes.json\n";
        return 1;
    }

    json data;
    try {
        file >> data;
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
        return 1;
    }

    if (!data.contains("shapes")) {
        std::cerr << "Error: JSON missing 'shapes' key\n";
        return 1;
    }

   
    auto shapes_data = data["shapes"];
    std::cout << "Available shapes:\n";
    for (auto it = shapes_data.begin(); it != shapes_data.end(); ++it)
        std::cout << " - " << it.key() << '\n';

    string choice;
    cout << "\nTotal shapes loaded: " << shapes_data.size() << "\n";
    cout << "Enter shape name: ";
    cin >> choice;

    if (!shapes_data.contains(choice)) {
        std::cerr << "Shape not found.\n";
        return 1;
    }

    auto shape_json = shapes_data[choice];

    Shape shape;
    shape.name = choice;
    shape.width = shape_json["size"]["w"];
    shape.height = shape_json["size"]["h"];

    for (auto& cell : shape_json["cells"]) {
        shape.cells.push_back({cell["x"], cell["y"]});
    }

    

    // ------------------------------------------------------------
    // CONFIGURATION SECTION
    // ------------------------------------------------------------
    // Each "cell" will be a square this many pixels wide/tall
    const int cellSize = params.contains("cell_size") ? (int)params["cell_size"] : 20;

    // // Number of cells horizontally and vertically
    // const int gridWidth  = 30;  // 30 cells across
    // const int gridHeight = 20;  // 20 cells tall

    // Total pixel dimensions 
    const int windowWidth  = params.contains("width") ? (int)params["width"] : 600;
    const int windowHeight = params.contains("height") ? (int)params["height"] : 400;

    // Grid dimensions 
    const int gridWidth = windowWidth / cellSize;
    const int gridHeight = windowHeight / cellSize;

    // Calculates center offset for the shapes
    int minX = shape.cells[0].x;
    int maxX = shape.cells[0].x;
    int minY = shape.cells[0].y;
    int maxY = shape.cells[0].y;

    // Loop through all cells to find the min and max x and y values
    for (const auto& cell : shape.cells) {
        minX = min(minX, cell.x);
        maxX = max(maxX, cell.x);
        minY = min(minY, cell.y);
        maxY = max(maxY, cell.y);
    }


    // Calculate the width and height of the box of the shapes
    int patternWidth = maxX - minX + 1;
    int patternHeight = maxY - minY + 1;
    int offsetX = (gridWidth - patternWidth) / 2 - minX;
    int offsetY = (gridHeight - patternHeight) / 2 - minY;
    // ------------------------------------------------------------
    // INITIALIZE SDL
    // ------------------------------------------------------------
    // SDL_Init starts the requested SDL subsystems.
    // SDL_INIT_VIDEO allows us to create a window and draw graphics.
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << "\n";
        return 1;  // Return non-zero to indicate an error
    }

    // ------------------------------------------------------------
    // CREATE THE WINDOW
    // ------------------------------------------------------------
    // SDL_CreateWindow opens a visible OS-level window.
    // Parameters:
    //   - Title text
    //   - x, y screen position (SDL_WINDOWPOS_CENTERED lets SDL decide)
    //   - Width and height in pixels
    //   - Flags (SDL_WINDOW_SHOWN = visible on creation)
    SDL_Window* window = SDL_CreateWindow("SDL2 Grid Example",     // title
                                          SDL_WINDOWPOS_CENTERED,  // x position
                                          SDL_WINDOWPOS_CENTERED,  // y position
                                          windowWidth,             // window width (pixels)
                                          windowHeight,            // window height (pixels)
                                          SDL_WINDOW_SHOWN         // flags
    );

    // Verify the window was successfully created
    if (!window) {
        std::cerr << "Window Error: " << SDL_GetError() << "\n";
        SDL_Quit();  // Clean up SDL before exiting
        return 1;
    }

    // ------------------------------------------------------------
    // CREATE A RENDERER
    // ------------------------------------------------------------
    // The renderer handles drawing operations on the window.
    // SDL_RENDERER_ACCELERATED tells SDL to use GPU acceleration.
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer Error: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // ------------------------------------------------------------
    // MAIN LOOP
    // ------------------------------------------------------------
    // "running" flag controls the lifetime of the program.
    bool running = true;
    SDL_Event event;  // Struct that holds event information (keyboard, mouse, quit, etc.)

    while (running) {
        // --------------------------------------------------------
        // EVENT HANDLING
        // --------------------------------------------------------
        // SDL_PollEvent() pulls events from the event queue.
        // This loop checks for any pending events, e.g. user clicking "X" to close.
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)  // Window close event
                running = false;         // Exit the main loop
            
            // Check for ESC key press
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                running = false;         // Exit when ESC is pressed
        }

        // --------------------------------------------------------
        // CLEAR SCREEN
        // --------------------------------------------------------
        // Set the background color first (dark blue-gray here).
        // The format is RGBA, each component 0–255.
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderClear(renderer);

        // --------------------------------------------------------
        // DRAW GRID LINES
        // --------------------------------------------------------
        // Set the color for the grid lines (lighter gray).
        SDL_SetRenderDrawColor(renderer, 80, 80, 100, 255);

        // Draw vertical lines.
        // Start at x = 0 and go to windowWidth, stepping by cellSize each time.
        for (int x = 0; x <= windowWidth; x += cellSize) {
            SDL_RenderDrawLine(renderer, x, 0, x, windowHeight);
        }

        // Draw horizontal lines.
        // Start at y = 0 and go to windowHeight, stepping by cellSize each time.
        for (int y = 0; y <= windowHeight; y += cellSize) {
            SDL_RenderDrawLine(renderer, 0, y, windowWidth, y);
        }

        
        // Sets color for the shapes
        srand(time(0));
        int r = rand() % 256;
        int g = rand() % 256;
        int b = rand() % 256;
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        
        // Draw each cell from the selected shape
        for (const auto& cell : shape.cells) {
            SDL_Rect cellRect;
            cellRect.x = (cell.x + offsetX) * cellSize;
            cellRect.y = (cell.y + offsetY) * cellSize;
            cellRect.w = cellSize;
            cellRect.h = cellSize;
            
            SDL_RenderFillRect(renderer, &cellRect);
        }

        // --------------------------------------------------------
        // SHOW THE RESULT
        // --------------------------------------------------------
        // Swap the off-screen buffer with the on-screen buffer.
        // Everything drawn since the last call to SDL_RenderPresent()
        // now becomes visible.
        SDL_RenderPresent(renderer);

        // --------------------------------------------------------
        // FRAME RATE LIMIT
        // --------------------------------------------------------
        // Delay ~16 ms to target roughly 60 frames per second.
        // (1000 ms / 60 ≈ 16.6 ms)
        SDL_Delay(16);
    }

    // ------------------------------------------------------------
    // CLEANUP
    // ------------------------------------------------------------
    // Free SDL resources before exiting to avoid memory leaks.
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;  // 0 = successful program termination
}