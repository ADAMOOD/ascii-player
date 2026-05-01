#pragma once
#include <string>
#include <vector>
#include "core/ConfigManager.h"
#include "strategies/StrategiesFactory.h"

class Tui
{
public:
    Tui() = default;
    ~Tui() = default;

    /**
     * @brief Zobrazi interaktivni menu a vrati index vybrane polozky.
     * @param options Seznam textu, ktere se maji v menu zobrazit.
     * @return int Index vybrane polozky.
     */
    int showMenu(const std::vector<std::string> &options, const std::string &menuTitle);
    std::string showFileExplorer(const std::string& directory);
    void showOptionsMenu();
};