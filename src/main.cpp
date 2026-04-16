#include "AsciiEngine.h"
#include "Tui.h"
#include <iostream>
#include <string>
#include <vector>

int main()
{
    // 1. Priprava dat (tady pozdeji pridas cteni slozky pres std::filesystem)
    std::vector<std::string> mainMenu = {
        "ASCII video convertor",
        "Optins",
        "[ Exit ]"};

    // 2. Inicializace a zobrazeni UI
    Tui tui;
    tui.init();

    // Predame data primo do metody
    int choiceIndex = tui.showMenu(mainMenu);

    // 3. Uklid po TUI
    std::cout << "\x1b[0m\x1b[2J\x1b[H";

    // 4. Vyhodnoceni volby
    if (choiceIndex == mainMenu.size() - 1)
    {
        std::cout << "Goodbye!\n";
        return 0;
    }

    switch (choiceIndex)
    {
    case 0:
    {
        tui.init();
         break;
    }

    default:
        break;
    }

    return 0;
}