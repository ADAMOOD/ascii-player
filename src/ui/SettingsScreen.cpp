#include "ui/SettingsScreen.h"
#include "ui/Tui.h"
#include <filesystem>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "ole32.lib")

std::vector<std::string> getAvailableCameras() 
{
    std::vector<std::string> cameras;
    CoInitializeEx(NULL, COINIT_MULTITHREADED); // Inicializace Windows COM

    IMFAttributes *pAttributes = NULL;
    HRESULT hr = MFCreateAttributes(&pAttributes, 1);
    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    }

    IMFActivate **ppDevices = NULL;
    UINT32 count = 0;
    if (SUCCEEDED(hr))
    {
        hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count); 
    }

    if (SUCCEEDED(hr))
    {
        for (UINT32 i = 0; i < count; i++)
        {
            WCHAR *szFriendlyName = NULL;
            UINT32 cchName;
            
            hr = ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &szFriendlyName, &cchName);
            if (SUCCEEDED(hr))
            {
                int size_needed = WideCharToMultiByte(CP_UTF8, 0, szFriendlyName, -1, NULL, 0, NULL, NULL);
                std::string name(size_needed, 0);
                WideCharToMultiByte(CP_UTF8, 0, szFriendlyName, -1, &name[0], size_needed, NULL, NULL);
                if (!name.empty() && name.back() == '\0')
                    name.pop_back();


                cameras.push_back(std::to_string(i) + ": " + name);
                CoTaskMemFree(szFriendlyName);
            }
            else
            {
                cameras.push_back(std::to_string(i) + ": Unknown Device");
            }
            ppDevices[i]->Release();
        }
        CoTaskMemFree(ppDevices);
    }
    if (pAttributes)
        pAttributes->Release();
    if (cameras.empty())
        cameras.push_back("0: Default Camera");
    return cameras;
}
#else
std::vector<std::string> getAvailableCameras()
{
    return {"0: Default Camera", "1: External Camera 1", "2: External Camera 2"};
}
#endif
bool SettingsScreen::save()
{
    bool success = true;
    if (m_use_webcam)
        success &= ConfigManager::saveVideoPath("webcam");
    else
        success &= ConfigManager::saveVideoPath(this->m_videoPath);

    success &= ConfigManager::setValToSettings("target_fps", m_target_fps);
    success &= ConfigManager::setValToSettings("render_strategy", m_allStrategies[this->m_selectedStrategyIndex]);
    success &= ConfigManager::saveFillChar(m_fill_char);
    success &= ConfigManager::setValToSettings("camera_index", std::to_string(m_selectedCameraIndex));
    return success;
}

void SettingsScreen::show(Tui &tui)
{
    using namespace ftxui;

    while (true)
    {
        auto screen = ScreenInteractive::Fullscreen();
        bool openExplorer = false;
        int selectedIndexMenu = 0;
        std::vector<std::string> menuEntries = {"Save settings", "Back to main menu"};
        MenuOption menuOption;
        std::string statusMessage = "";

        menuOption.on_enter = [&]
        {
            if (selectedIndexMenu == 0)
            {
                if (save())
                    statusMessage = "Success: Settings saved!";
                else
                    statusMessage = "Error: Invalid parameters. Please check your inputs.";
            }
            else if (selectedIndexMenu == 1)
            {
                screen.ExitLoopClosure()();
            }
        };

        // --- COMPONENTS (LOGIC) ---
        Component inputPath = Input(&m_videoPath, "please select a video file");
        Component browseButton = Button("Choose video from explorer", [&]
                                        {
            openExplorer = true;
            screen.ExitLoopClosure()(); });
        Component inputFps = Input(&m_target_fps, "please enter target FPS");
        Component inputChar = Input(&m_fill_char, "please enter fill character for edge strategies");
        Component strategyDropdown = Dropdown(&m_allStrategies, &m_selectedStrategyIndex);
        Component menuButtons = Menu(&menuEntries, &selectedIndexMenu, menuOption);


        ftxui::CheckboxOption cbOpt = ftxui::CheckboxOption::Simple();
        cbOpt.transform = [](const ftxui::EntryState &s)
        {
            auto prefix = ftxui::text(s.state ? "[X] " : "[ ] ");
            auto t = ftxui::text(s.label);
            if (s.active)
                t |= ftxui::inverted;
            return ftxui::hbox({prefix, t});
        };

        Component webcamCheckbox = Checkbox("Use webcam as a video source", &m_use_webcam, cbOpt);
        Component cameraDropdown = Dropdown(&m_cameraNames, &m_selectedCameraIndex);

        auto container = Container::Vertical({
            inputPath,
            browseButton,
            webcamCheckbox,
            cameraDropdown,
            inputFps,
            inputChar,
            strategyDropdown,
            menuButtons,
        });

        // --- RENDERER ---
        auto renderer = Renderer(container, [&]
                                 {    
            Element videoRow = hbox({text(" Video:          ") | color(Color::GrayLight), inputPath->Render()});
            Element browseRow = hbox({text("                 "), browseButton->Render()}); // Odsazení pro tlačítko
            Element webcamRow = hbox({text(" Webcam:         ") | color(Color::GrayLight), webcamCheckbox->Render()});
            Element fpsRow   = hbox({text(" Target FPS:     ") | color(Color::GrayLight), inputFps->Render()});
            Element charRow  = hbox({text(" Fill Character: ") | color(Color::GrayLight), inputChar->Render()});
            Element stratRow = hbox({text(" Strategy:       ") | color(Color::GrayLight), strategyDropdown->Render()});
            Element indexRow = hbox({text(" Source Camera:  ") | color(Color::GrayLight), cameraDropdown->Render()});
            
            Element content = vbox({
                videoRow,
                browseRow,
                webcamRow,
                indexRow,
                fpsRow,
                charRow,
                stratRow,
                separator(),
                text(statusMessage) | center, 
                separator(),
                menuButtons->Render(),
            });
            
            return window(text(" Settings ") | bold | center, content) | center; });

        // loop for user interaction with the settings screen
        screen.Loop(renderer);

        //-- AFTER LOOP (WHEN USER EXITS THE SETTINGS SCREEN) ---

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

            std::string selectedFile = tui.showFileExplorer(startDir);

            if (!selectedFile.empty())
            {
                m_videoPath = selectedFile;
            }
        }
        else
        {
            // escape or Back to main menu was selected
            break;
        }
    }
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

    this->m_cameraNames = getAvailableCameras();
    
    std::string camStr = ConfigManager::getValFromSettings("camera_index");
    if (!camStr.empty()) {
        try { m_selectedCameraIndex = std::stoi(camStr); } catch (...) { m_selectedCameraIndex = 0; }
    }
    if (m_selectedCameraIndex < 0 || m_selectedCameraIndex >= static_cast<int>(m_cameraNames.size())) {
        m_selectedCameraIndex = 0;
    }
}