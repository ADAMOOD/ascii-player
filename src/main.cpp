    #include "AsciiEngine.h"
    #include "Tui.h"
    #include "ConfigManager.h"
    #include <iostream>
    #include <string>
    #include <vector>

int main()
{
    std::string currentVideo = ConfigManager::loadVideoPath();

    std::vector<std::string> mainMenu = {
        "ASCII video convertor",
        "Showcase",
        "Options",
        "[ Exit ]"
    };

    Tui tui;
    

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
            if(currentVideo.empty())
            {
                currentVideo = tui.showFileExplorer("../");
                ConfigManager::saveVideoPath(currentVideo);
            }
            break;
        }
        case 1:
        {

            
            AsciiEngine engine;
            if (engine.init("../assets/testEarth.mp4"))
            {
                engine.play();
            }
            else
            {
                std::cerr << "[CHYBA] Nepodarilo se inicializovat engine." << std::endl;
            }
            break;
        }
    }

    return 0;
}