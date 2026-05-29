#pragma once
#include <string>
#include <vector>


/**
 * @brief Text user interface (TUI) class responsible for displaying menus, file explorers, and the settings screen.
 * 
 */
class Tui
{
public:
    Tui() = default;
    ~Tui() = default;

    /**
     * @brief Displays a menu with the given options and title, allowing the user to navigate and select an option.
     * 
     * @param options = List of menu options to display.
     * @param menuTitle = Title to display at the top of the menu.
     * @return int Index of the selected option in the options vector.
     */
    int showMenu(const std::vector<std::string> &options, const std::string &menuTitle);
    /**
     * @brief Displays a file explorer starting at the specified directory, allowing the user to navigate and select a video file. Only supported video formats are shown.
     * 
     * @param directory Starting directory for the file explorer.
     * @return std::string Absolute path of the selected video file, or an empty string if the user cancels the selection.
     */
    std::string showFileExplorer(const std::string &directory);
    /**
     * @brief Displays the settings screen, allowing the user to adjust preferences such as video path, target FPS, fill character for edge strategies, and the rendering strategy itself. Interacts with ConfigManager to load and save settings.
     * 
     */
    void showOptionsMenu();
};