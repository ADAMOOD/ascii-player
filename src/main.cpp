/**
 * @file main.cpp
 * @brief Entry point for the ARTSCII application.
 * * Initializes the user interface, handles the main menu loop, and manages 
 * the lifecycle of the AsciiEngine for both video files and webcam input.
 */

#include "core/AsciiEngine.h"
#include "ui/Tui.h"
#include "core/ConfigManager.h"
#include <iostream>
#include <string>
#include <vector>
#include <opencv2/core/utils/logger.hpp> 

/**
 * @brief Main execution function.
 * @return int Exit status (0 for success).
 */
int main()
{
    // ensure OpenCV logging is disabled to prevent cluttering the terminal output
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);

    std::vector<std::string> mainMenu = {
        "ASCII video convertor",
        "Options",
        "[ Exit ]"
    };

    Tui tui;
    
    // main App loop
    while (true)
    {
        int choiceIndex = tui.showMenu(mainMenu, "ARTSCII");

        if ((size_t)choiceIndex == mainMenu.size() - 1)
        {
            std::cout << "Goodbye!\n";
            return 0;
        }

        switch (choiceIndex)
        {
        case 0: // ASCII video convertor
        {
            bool useWebcam = ConfigManager::GetUseWebcam();
            AsciiEngine engine;
            
            if (!useWebcam)
            {
                // video playback from file
                std::string currentVideo = ConfigManager::loadVideoPath();
                if (currentVideo.empty())
                {
                    // Fallback to file explorer if no valid path is found in settings
                    currentVideo = tui.showFileExplorer("../");
                    ConfigManager::saveVideoPath(currentVideo);
                }

                if (engine.init(currentVideo))
                {
                    engine.play();
                }
                else
                {
                    std::cerr << "[ERROR] Failed to start video playback." << std::endl;
                }
            }
            else
            {
                // Webcam live rendering
                if (engine.init())
                {
                    engine.play();
                }
                else
                {
                    std::cerr << "[ERROR] Failed to start webcam." << std::endl;
                }
            }
            break;
        }
        case 1: // settings
        {
            tui.showOptionsMenu();
            break;
        }
        }
    }

    return 0;
}