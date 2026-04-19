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

    std::vector<std::string> options = {"up"};
    std::vector<std::string> allowedFileFormats = {".mp4",
                                                   ".mov",
                                                   ".avi"};
    int i = 0;
    for (const auto &file : std::filesystem::directory_iterator(directory))
    {
        if (file.is_regular_file() && std::find(allowedFileFormats.begin(), allowedFileFormats.end(), file.path().extension()) != allowedFileFormats.end())
        {
            options.push_back(file.path().filename().string());
            i++;
        }
    }
    if(i==0)
    {
        
    }
    this->showMenu(options, "Choose a video file to convert");
    return "";
}