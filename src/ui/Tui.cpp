#include "ui/Tui.h"
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
    // convert to absolute path and normalize it to avoid issues with relative paths and redundant path components
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
void Tui::showOptionsMenu()
{
    using namespace ftxui;
    auto screen = ScreenInteractive::Fullscreen();

    std::string videoPath = ConfigManager::loadVideoPath();
    std::string targetFps = "30";
    int selectedIndex = 0;
    std::vector<std::string> menuEntries = {"Save settings", "Back to main menu"};

    MenuOption menuOption;
    menuOption.on_enter = screen.ExitLoopClosure();

    // Vytvoření jednotlivých komponent
    Component inputIP = Input(&videoPath, "please select a video file");
    Component inputUser = Input(&targetFps, "please enter target FPS");
    // 2. NEZAPOMEŇ PŘEDAT menuOption!
    Component menuButtons = Menu(&menuEntries, &selectedIndex, menuOption);

    // Sestavení do vertikálního kontejneru
    auto container = Container::Vertical({
        inputIP,
        inputUser,
        menuButtons,
    });

    // Renderer pro definici vzhledu
    auto renderer = Renderer(container, [&]
                             { return window(text(" Settings ") | bold | center,
                                             vbox({
                                                 hbox(text(" Video:  "), inputIP->Render()),
                                                 hbox(text(" FPS:    "), inputUser->Render()),
                                                 separator(),
                                                 menuButtons->Render(),
                                             })) |
                                      center; });

    screen.Loop(renderer);

    // 4. SEM SE PROGRAM DOSTANE AŽ PO STISKNUTÍ ENTERU NAD NĚJAKÝM TLAČÍTKEM
    if (selectedIndex == 0) // Uživatel klikl na "Save settings"
    {
        ConfigManager::saveVideoPath(videoPath);
        // Sem pak můžeš přidat ukládání FPS atd.
    }
    return;
}