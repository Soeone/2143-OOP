#include <sys/ioctl.h>
#include <unistd.h>  // For STDOUT_FILENO
#include <iostream>
#include <SDL2/SDL.h>

// Project headers
#include "./includes/AutomatonUtils.hpp"
#include "./includes/ConwayLife.hpp"
#include "./includes/Screen.hpp"
#include "./includes/argsToJson.hpp"
#include "./includes/json.hpp"
#include "./includes/CellularAutomaton.hpp"
#include "./includes/Click.hpp"

using namespace std;
using nlohmann::json;

// --------------------------------------------------------------
// Default simulation parameters.
// These values are applied *only if* the user does not provide
// command-line overrides of the form key=value.
// --------------------------------------------------------------
json defaults = {{"width", 800}, {"height", 600}, {"generations", 1000}, {"cellSize", 10}, {"frameDelayMs", 500}};

int main(int argc, char* argv[]) {

     
    json params = ArgsToJson(argc, argv);

    // ----------------------------------------------------------
    // Merge defaults: if a parameter wasn't specified by the user,
    // use the value from `defaults`.
    //
    // Result:
    //   - user-provided params overwrite defaults
    //   - missing keys fall back to defaults
    // ----------------------------------------------------------
    for (auto& [key, value] : defaults.items()) {
        if (params.find(key) == params.end()) {
            params[key] = value;
        }
    }

    cout << "Simulation Parameters:\n"
         << params.dump(4)  // pretty-printed JSON
         << endl;

    // ----------------------------------------------------------
    // Determine the terminal window size.
    // Using ioctl() with TIOCGWINSZ retrieves:
    //   - number of character rows (w.ws_row)
    //   - number of character columns (w.ws_col)
    //
    // This allows the simulation to scale to the user's terminal.
    // ----------------------------------------------------------
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        std::cout << "Terminal rows:    " << w.ws_row << std::endl;
        std::cout << "Terminal columns: " << w.ws_col << std::endl;
    } else {
        std::cerr << "Error getting terminal size." << std::endl;
    }

    // ----------------------------------------------------------
    // TextScreen implements the Screen interface using simple
    // Unicode/ASCII-based rendering in the terminal.
    // ----------------------------------------------------------
   SdlScreen screen(params["width"], params["height"], params["cellSize"]);

    // ----------------------------------------------------------
    // Construct a ConwayLife automaton based on available space.
    //
    // Terminal columns are divided by 2 because each rendered cell
    // uses two characters ("⬜" or two spaces). Without dividing,
    // the grid would overflow horizontally.
    //
    // Subtracting 1 row prevents scrolling due to newline behavior.
    // ----------------------------------------------------------
    ConwayLife gol(w.ws_row - 1, w.ws_col / 2);

    // ----------------------------------------------------------
    // Main simulation loop.
    // This runs indefinitely:
    //   1. Render current grid
    //   2. Advance one generation (step)
    //   3. Pause for a fixed delay
    //
    // ctrl-C exits.
    // ----------------------------------------------------------
    Click click;
    bool running = true;

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            click.handleEvent(e);
            
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }

        if (click.leftClicked()) {
            std::cout << "Clicked at: "
                      << click.x() << ", " << click.y() << "\n";
        }

        SDL_Rect button{ 100, 100, 200, 100 };
        if (click.leftClicked() && click.inside(button)) {
            std::cout << "Button pressed!\n";
        }

        screen.render(gol.getGrid());
        gol.step();
        screen.pause(500);
    }

    return 0;
}