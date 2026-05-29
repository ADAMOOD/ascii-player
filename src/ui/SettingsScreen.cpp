#include "ui/SettingsScreen.h"
#include "ui/Tui.h"           // TOHLE TI TAM CHYBĚLO!
#include <filesystem>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>



bool SettingsScreen::save()
{
    bool success = true;
    if (m_use_webcam) success &= ConfigManager::saveVideoPath("webcam");
    else success &= ConfigManager::saveVideoPath(this->m_videoPath);

    success &= ConfigManager::setValToSettings("target_fps", m_target_fps);
    success &= ConfigManager::setValToSettings("render_strategy", m_allStrategies[this->m_selectedStrategyIndex]);
    success &= ConfigManager::saveFillChar(m_fill_char);
    return success;
}

void SettingsScreen::show(Tui& tui)
{
    using namespace ftxui;

    // VŠE MUSÍ BÝT VE SMYČCE
    while (true)
    {
        auto screen = ScreenInteractive::Fullscreen();
        bool openExplorer = false; // Vlajka pro tlačítko
        int selectedIndexMenu = 0;
        std::vector<std::string> menuEntries = {"Save settings", "Back to main menu"};
        MenuOption menuOption;
        std::string statusMessage = "";

        menuOption.on_enter = [&]
        {
            if (selectedIndexMenu == 0)
            {
                if (save()) statusMessage = "Success: Settings saved!";
                else statusMessage = "Error: Invalid FPS or File path!";
            }
            else if (selectedIndexMenu == 1)
            {
                screen.ExitLoopClosure()();
            }
        };

        // --- KOMPONENTY ---
        Component inputPath = Input(&m_videoPath, "please select a video file");
        
        // TLAČÍTKO JEN ZVEDNE VLAJKU A ZAVŘE MENU
        Component browseButton = Button("Choose video from explorer", [&] {
            openExplorer = true;
            screen.ExitLoopClosure()(); 
        });

        Component inputFps = Input(&m_target_fps, "please enter target FPS");
        Component inputChar = Input(&m_fill_char, "please enter fill character for edge strategies");
        Component strategyDropdown = Dropdown(&m_allStrategies, &m_selectedStrategyIndex);
        Component menuButtons = Menu(&menuEntries, &selectedIndexMenu, menuOption);

        ftxui::CheckboxOption cbOpt = ftxui::CheckboxOption::Simple();
        cbOpt.transform = [](const ftxui::EntryState &s)
        {
            auto prefix = ftxui::text(s.state ? "[X] " : "[ ] ");
            auto t = ftxui::text(s.label);
            if (s.active) t |= ftxui::inverted;
            return ftxui::hbox({prefix, t});
        };

        Component webcamCheckbox = Checkbox("Use webcam as a video source", &m_use_webcam, cbOpt);

        // --- KONTEJNER (Teď už i s tlačítkem) ---
        auto container = Container::Vertical({
            inputPath,
            browseButton, // Přidáno sem!
            webcamCheckbox,
            inputFps,
            inputChar,
            strategyDropdown,
            menuButtons,
        });

        // --- VYKRESLENÍ ---
        auto renderer = Renderer(container, [&] {    
            Element videoRow = hbox({text(" Video:          ") | color(Color::GrayLight), inputPath->Render()});
            Element browseRow = hbox({text("                 "), browseButton->Render()}); // Odsazení pro tlačítko
            Element webcamRow = hbox({text(" Webcam:         ") | color(Color::GrayLight), webcamCheckbox->Render()});
            Element fpsRow   = hbox({text(" Target FPS:     ") | color(Color::GrayLight), inputFps->Render()});
            Element charRow  = hbox({text(" Fill Character: ") | color(Color::GrayLight), inputChar->Render()});
            Element stratRow = hbox({text(" Strategy:       ") | color(Color::GrayLight), strategyDropdown->Render()});
            
            Element content = vbox({
                videoRow,
                browseRow, // Přidáno sem!
                webcamRow,
                fpsRow,
                charRow,
                stratRow,
                separator(),
                text(statusMessage) | center, 
                separator(),
                menuButtons->Render(),
            });
            
            return window(text(" Settings ") | bold | center, content) | center; 
        });

        // Zamrzne a čeká na uživatele
        screen.Loop(renderer);

        // --- CO SE STANE PO ZAVŘENÍ OKNA ---
        if (openExplorer)
        {
            std::string startDir = "./";
            if (!m_videoPath.empty()) 
            {
                std::filesystem::path p(m_videoPath);
                if (p.has_parent_path() && std::filesystem::exists(p.parent_path())) 
                {
                    startDir = p.parent_path().string();
                }
            }

            // Zavoláme explorer z Tui
            std::string selectedFile = tui.showFileExplorer(startDir);
            
            if (!selectedFile.empty()) 
            {
                m_videoPath = selectedFile; // Přepíšeme cestu
            }
            // Záměrně tu není break -> smyčka while(true) se otočí a okno se ukáže s novou cestou!
        }
        else
        {
            // Bylo zmáčknuto Back To Menu (nebo ESC), jdeme úplně pryč
            break;
        }
    }
}

// Konstruktor zůstává úplně stejný, jak jsi ho měl ty!
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
    auto distance = std::find(m_allStrategies.begin(), m_allStrategies.end(), savedStrategy);
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