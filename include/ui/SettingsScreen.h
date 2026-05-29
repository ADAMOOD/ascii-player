#pragma once
#include <vector>
#include <string>
#include "strategies/StrategiesFactory.h"
#include "core/ConfigManager.h"
#include <algorithm>

// Slib kompilátoru, že Tui existuje (vyhneme se kruhové závislosti)
class Tui; 

class SettingsScreen
{
private:
    std::string m_videoPath;
    std::string m_target_fps;
    std::string m_fill_char;
    bool m_use_webcam;

    std::vector<std::string> m_allStrategies; 
    int m_selectedStrategyIndex;              
    int m_selectedIndexMenu;

    bool save();

public:
    void show(Tui& tui); // Zde přijímáme referenci na Tui
    SettingsScreen();
    ~SettingsScreen() = default;
};