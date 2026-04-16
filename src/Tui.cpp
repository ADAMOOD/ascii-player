#include "Tui.h"
#include <iostream>

// FTXUI Headers
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

void Tui::init()
{
    // Clear terminal before showing UI
    std::cout << "\x1b[2J\x1b[H";
}

int Tui::showMenu(const std::vector<std::string>& options)
{
    using namespace ftxui;

    int selectedIndex = 0;
    auto screen = ScreenInteractive::TerminalOutput();

    MenuOption menuOption;
    menuOption.on_enter = screen.ExitLoopClosure();

    // Tady pouzivame primo ty moznosti, ktere prisly v parametru
    Component menu = Menu(&options, &selectedIndex, menuOption);

    Component renderer = Renderer(menu, [&] {
        return window(
            text(" ASCII PLAYER") | bold | center,
            menu->Render()
        ) | center;
    });

    screen.Loop(renderer);

    return selectedIndex;
}