#pragma once
#include <string>
#include <vector>

class Tui
{
public:
    Tui() = default;
    ~Tui() = default;

    int showMenu(const std::vector<std::string> &options, const std::string &menuTitle);
    std::string showFileExplorer(const std::string &directory);
    void showOptionsMenu();
};