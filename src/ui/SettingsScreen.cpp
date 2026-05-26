#include "ui/SettingsScreen.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
bool SettingsScreen::save()
{
    bool success = true;
    if (m_use_webcam)
    {
        success &= ConfigManager::saveVideoPath("webcam");
    }
    else
    {
        success &= ConfigManager::saveVideoPath(this->m_videoPath);
    }

    success &= ConfigManager::setValToSettings("target_fps", m_target_fps);
    success &= ConfigManager::setValToSettings("render_strategy", m_allStrategies[this->m_selectedStrategyIndex]);
    success &= ConfigManager::saveFillChar(m_fill_char);
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
    Component inputPath = Input(&m_videoPath, "please select a video file");
    Component inputFps = Input(&m_target_fps, "please enter target FPS");
    Component inputChar = Input(&m_fill_char, "please enter fill character for edge strategies");
    Component strategyDropdown = Dropdown(&m_allStrategies, &m_selectedStrategyIndex);
    Component menuButtons = Menu(&menuEntries, &selectedIndexMenu, menuOption);

    ftxui::CheckboxOption cbOpt = ftxui::CheckboxOption::Simple();
    cbOpt.transform = [](const ftxui::EntryState &s)
    {
        // s.state obsahuje true/false podle toho, jestli je checkbox zaškrtnutý
        auto prefix = ftxui::text(s.state ? "[X] " : "[ ] ");
        auto t = ftxui::text(s.label);

        // Zvýraznění celého řádku, když na něj najedeš šipkami
        if (s.active)
        {
            t |= ftxui::inverted;
        }
        return ftxui::hbox({prefix, t});
    };

    Component webcamCheckbox = Checkbox("Use webcam as a video source", &m_use_webcam, cbOpt);

    // 2. Propojení LOGIKY pro navigaci šipkami
    auto container = Container::Vertical({
        inputPath,
        webcamCheckbox,
        inputFps,
        inputChar,
        strategyDropdown,
        menuButtons,
    });

    // 3. VYKRESLOVÁNÍ (Rozdělené na menší, čitelné kusy)
    auto renderer = Renderer(container, [&]
                             {    
    // Poskládáme si jednotlivé řádky
Element videoRow = hbox({text(" Video:          ") | color(Color::GrayLight), inputPath->Render()});
Element webcamRow = hbox({text(" Webcam:         ") | color(Color::GrayLight), webcamCheckbox->Render()});
Element fpsRow   = hbox({text(" Target FPS:     ") | color(Color::GrayLight), inputFps->Render()});
Element charRow  = hbox({text(" Fill Character: ") | color(Color::GrayLight), inputChar->Render()});
Element stratRow = hbox({text(" Strategy:       ") | color(Color::GrayLight), strategyDropdown->Render()});
    
    // Sloučíme řádky a tlačítka pod sebe
    Element content = vbox({
        videoRow,
        webcamRow,
        fpsRow,
        charRow,
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
    this->m_fill_char = ConfigManager::getValFromSettings("fill_char");
    if (m_fill_char.empty() || m_fill_char.length() != 1)
    {
        m_fill_char = ".";
    }
    m_use_webcam = ConfigManager::GetUseWebcam();
}