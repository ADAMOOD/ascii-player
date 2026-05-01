#include "ui/SettingsScreen.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
bool SettingsScreen::save()
{
    bool success = true;
    success &= ConfigManager::saveVideoPath(this->m_videoPath);
    success &= ConfigManager::setValToSettings("target_fps", m_target_fps);
    success &= ConfigManager::setValToSettings("render_strategy", m_allStrategies[this->m_selectedStrategyIndex]);
    return success;
}
void SettingsScreen::show()
{
    using namespace ftxui;
    auto screen = ScreenInteractive::Fullscreen();

    int selectedIndexMenu = 0;
    std::vector<std::string> menuEntries = {"Save settings", "Back to main menu"};
    MenuOption menuOption;
    std::string statusMessage = ""; 

    menuOption.on_enter = [&]
    {
        if (selectedIndexMenu == 0) // save settings
        {
            if (save())
            {
                statusMessage = "Success: Settings saved!";
            }
            else
            {
                statusMessage = "Error: Invalid FPS or File path!";
            }
        }
        else if (selectedIndexMenu == 1) // back to main menu
        {
            // user choose to go back to main menu, we just exit the settings screen without saving
            screen.ExitLoopClosure()();
        }
    };

    // 1. Vytvoření LOGIKY (Komponenty)
    Component inputIP = Input(&m_videoPath, "please select a video file");
    Component inputUser = Input(&m_target_fps, "please enter target FPS");
    Component strategyDropdown = Dropdown(&m_allStrategies, &m_selectedStrategyIndex);
    Component menuButtons = Menu(&menuEntries, &selectedIndexMenu, menuOption);

    // 2. Propojení LOGIKY pro navigaci šipkami
    auto container = Container::Vertical({
        inputIP,
        inputUser,
        strategyDropdown,
        menuButtons,
    });

    // 3. VYKRESLOVÁNÍ (Rozdělené na menší, čitelné kusy)
    auto renderer = Renderer(container, [&]
                             {    
    // Poskládáme si jednotlivé řádky
    Element videoRow = hbox({text(" Video:    "), inputIP->Render()});
    Element fpsRow   = hbox({text(" FPS:      "), inputUser->Render()});
    Element stratRow = hbox({text(" Strategy: "), strategyDropdown->Render()});
    
    // Sloučíme řádky a tlačítka pod sebe
    Element content = vbox({
        videoRow,
        fpsRow,
        stratRow,
        separator(),
        // Tady se vypíše status. Můžeš přidat i barvu přes | color(Color::Red)
        text(statusMessage) | center, 
        separator(),
        menuButtons->Render(),
    });
    
    // Obalíme to oknem a vycentrujeme
    return window(text(" Settings ") | bold | center, content) | center; });
    screen.Loop(renderer);
    return;
}
SettingsScreen::SettingsScreen()
{
    this->m_allStrategies = StrategiesFactory::getAvailableStrategies();
    this->m_target_fps = ConfigManager::getValFromSettings("target_fps");
    if (m_target_fps.empty())
    {
        m_target_fps = "30";
    }
    this->m_videoPath = ConfigManager::loadVideoPath();
    std::string savedStrategy = ConfigManager::getValFromSettings("render_strategy");
    auto distance = std::find(m_allStrategies.begin(), m_allStrategies.end(), savedStrategy); // find returns distance of found item in array
    if (distance == m_allStrategies.end())
    {
        this->m_selectedStrategyIndex = 0;
    }
    else
    {
        this->m_selectedStrategyIndex = std::distance(m_allStrategies.begin(), distance);
    }
}