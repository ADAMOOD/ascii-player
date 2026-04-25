#include "core/AsciiEngine.h"
#include "ui/Tui.h"
#include "core/ConfigManager.h"
#include <iostream>
#include <string>
#include <vector>

int main()
{
    std::string currentVideo = ConfigManager::loadVideoPath();

    std::vector<std::string> mainMenu = {
        "ASCII video convertor",
        "Options",
        "[ Exit ]"};

    Tui tui;
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
        case 0:
        {
            if (currentVideo.empty())
            {
                currentVideo = tui.showFileExplorer("../");
                ConfigManager::saveVideoPath(currentVideo);
            }
            AsciiEngine engine;
        
            if (engine.init(currentVideo))
            {
                engine.play();
            }
            else
            {
                std::cerr << "[ERR] " << std::endl;
            }
            break;
        }
        case 1:
        {
            tui.showOptionsMenu();
            break;
        }
        }
    }

    return 0;
}