#pragma once
#include <vector>
#include <string>
#include "strategies/StrategiesFactory.h"
#include "core/ConfigManager.h"
#include <algorithm>
class SettingsScreen
{
private:
    std::string m_videoPath;
    std::string  m_target_fps;

    std::vector<std::string> m_allStrategies; // Seznam pro rozbalovací menu
    int m_selectedStrategyIndex;              // Ukazatel na to, co je zrovna vybráno

    int m_selectedIndexMenu;

    bool save();

public:
    void show();
    SettingsScreen();
    ~SettingsScreen() = default;
};


