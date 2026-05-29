#pragma once
#include <vector>
#include <string>
#include "strategies/StrategiesFactory.h"
#include "core/ConfigManager.h"
#include <algorithm>

/**
 * @brief class for managing the settings screen of the application.
 *  It provides an interface for users to adjust preferences such as video path, target FPS, fill character for edge strategies, and the rendering strategy itself.
 *  The class interacts with the Tui component to display menus and handle user input, and it uses ConfigManager to persist changes to a configuration file.
 * 
 */
class Tui; 

class SettingsScreen
{
private:
    std::string m_videoPath;///< Path to the video file or "webcam" for live feed.
    std::string m_target_fps;///< Desired frames per second for playback (stored as string for direct binding to input field).(NOT IMPLEMENTED YET)
    std::string m_fill_char;///< Character used for filling edge detection blank areas.
    bool m_use_webcam;///< Flag indicating whether to use the webcam.

    std::vector<std::string> m_allStrategies; ///< List of all available rendering strategies for the dropdown menu.
    int m_selectedStrategyIndex;///< Index of the currently selected rendering strategy in the dropdown menu.
    int m_selectedIndexMenu;///< Index of the currently selected menu option (Save settings, Back to main menu).

    /**
     * @brief Saves the current settings to the configuration file using ConfigManager. Validates inputs and returns success status.
     * 
     * @return true = Settings were successfully saved.
     * @return false = There was an error saving settings (e.g., invalid video path, unsupported fill character).
     */
    bool save();

public:
    /**
     * @brief Displays the settings screen and handles user interactions.
     * 
     * @param tui Reference to the Tui component for file browsing and menu display.
     */
    void show(Tui& tui); 
    /**
     * @brief Constructs a new SettingsScreen object.
     * Initializes member variables with current configuration values and available strategies.
     */
    SettingsScreen();
    ~SettingsScreen() = default;
};