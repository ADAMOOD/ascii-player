#include "AsciiEngine.h"
#include <iostream>
#include <string>
#include <vector>

// FTXUI Headers
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

/**
 * @brief Displays a terminal UI menu for the user to select a video file.
 * @return std::string The selected filename, or empty string if user exited.
 */
std::string showVideoSelectionMenu() 
{
    using namespace ftxui;

    // 1. Data model: List of files and current selection index
    std::vector<std::string> videoFiles = {
        "Show video ASCII - ART",
        "[ Exit Program ]"
    };
    int selectedIndex = 0;

    // 2. Setup the screen object (this controls the raw terminal mode)
    auto screen = ScreenInteractive::TerminalOutput();

    // 3. Define behavior: What happens when the user presses Enter?
    MenuOption option;
    option.on_enter = screen.ExitLoopClosure(); // This breaks the screen.Loop()

    // 4. Create the interactive menu component
    Component menu = Menu(&videoFiles, &selectedIndex, option);

    // 5. Wrap the menu in a visually appealing DOM structure (Renderer)
    // The renderer is called every time the screen needs to redraw (e.g. arrow pressed)
    Component renderer = Renderer(menu, [&] {
        return window(
            text(" ASCII PLAYER - SELECT VIDEO ") | bold | center, // Window title
            menu->Render() // The actual interactive menu
        ) | center; // Center the whole window in the middle of the terminal
    });

    // 6. Start the blocking UI loop. Program pauses here until ExitLoopClosure is called.
    screen.Loop(renderer);

    // 7. Check what the user selected
    if (selectedIndex == videoFiles.size() - 1) {
        return ""; // User selected [ Exit Program ]
    }

    return videoFiles[selectedIndex];
}

int main() 
{
    // Clear terminal before showing UI
    std::cout << "\x1b[2J\x1b[H"; 

    // Show the menu and block until user makes a choice
    std::string chosenVideo = showVideoSelectionMenu();

    if (chosenVideo.empty()) {
        std::cout << "Goodbye!\n";
        return 0;
    }

    // Now start the engine with the chosen video
    AsciiEngine engine;
    if (engine.init(chosenVideo)) {
        engine.play();
    } else {
        std::cerr << "Failed to initialize engine with video: " << chosenVideo << std::endl;
    }

    // Reset terminal style after video finishes
    std::cout << "\x1b[0m\x1b[2J\x1b[H"; 
    
    return 0;
}