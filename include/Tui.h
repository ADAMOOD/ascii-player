#pragma once
#include <string>
#include <vector>

class Tui
{
public:
    Tui() = default;
    ~Tui() = default;

    /**
     * @brief Vycisti terminal a pripravi ho pro UI.
     */
    void init();

    /**
     * @brief Zobrazi interaktivni menu a vrati index vybrane polozky.
     * @param options Seznam textu, ktere se maji v menu zobrazit.
     * @return int Index vybrane polozky.
     */
    int showMenu(const std::vector<std::string>& options);
    std::string showFileExplorer(const std::string& directory);
};