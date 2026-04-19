#include "Tui.h"
#include <iostream>
#include <fstream>
#include <filesystem>

// FTXUI Headers
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

int Tui::showMenu(const std::vector<std::string> &options, const std::string &menuTitle)
{
    using namespace ftxui;

    int selectedIndex = 0;
    auto screen = ScreenInteractive::Fullscreen();

    MenuOption menuOption;
    menuOption.on_enter = screen.ExitLoopClosure();

    // Tady pouzivame primo ty moznosti, ktere prisly v parametru
    Component menu = Menu(&options, &selectedIndex, menuOption);

    Component renderer = Renderer(menu, [&]
                                  { return window(
                                               text(menuTitle) | bold | center,
                                               menu->Render()) |
                                           center; });

    screen.Loop(renderer);

    return selectedIndex;
}

std::string Tui::showFileExplorer(const std::string &directory)
{
    //convert to absolute path and normalize it to avoid issues with relative paths and redundant path components
    std::string currentDir = std::filesystem::absolute(directory).lexically_normal().string();
    while (true)
    {
        std::vector<std::string> options = {" ↑ .."};
        for (const auto &file : std::filesystem::directory_iterator(currentDir))
        {
            if (file.is_directory()) 
            {
                options.push_back(file.path().filename().string() + "/");
            }
            else if (file.is_regular_file() && 
                     ConfigManager::isSupportedVideoFormat(file.path().extension().string()))
            {
                options.push_back(file.path().filename().string());
            }
        }

        int choiceIndex = this->showMenu(options, "Choose a video file to convert");
        if (choiceIndex == 0) 
        {
            // parent_path() returns the path to the parent directory, effectively moving us "up" one level in the directory structure
            currentDir = std::filesystem::path(currentDir).parent_path().string();
        }
        else 
        {
            std::string selectedItem = options[choiceIndex];
            
            // operator / allows us to easily concatenate paths, taking care of the correct path separators for the operating system
            std::filesystem::path fullPath = std::filesystem::path(currentDir) / selectedItem;

            if (std::filesystem::is_directory(fullPath))
            {
                currentDir = fullPath.string(); 
            }
            else if (std::filesystem::is_regular_file(fullPath))
            {
                return fullPath.string(); 
            }
        }
    }
}