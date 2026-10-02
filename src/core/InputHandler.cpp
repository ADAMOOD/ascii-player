#include "core/InputHandler.h"

// --- OS dependent libraries  ---
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

InputAction InputHandler::pollInput()
{
    char c = 0;
    bool hasInput = false;

// --- MULTIPLATFORM ---
#ifdef _WIN32
    if (_kbhit())
    {
        c = _getch();
        hasInput = true;
    }
#else
    if (read(STDIN_FILENO, &c, 1) == 1)
    {
        hasInput = true;
    }
#endif

    if (hasInput)
    {
        if (c == 'q' || c == 'Q')
        {
            return InputAction::QUIT;
        }
        switch (c)
        {
        case 'a':
        {
            return InputAction::NEXT_PROPERTY;
        }

        case 'd':
        {
            return InputAction::PREV_PROPERTY;
        }

        case 'w':
        {
            return InputAction::INCREASE_VALUE;
        }
        case 's':
        {
            return InputAction::DECREASE_VALUE;
        }
        }
        return InputAction::NONE;
    }
}

bool InputHandler::init()
{
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif

    return true;
}

bool InputHandler::shutdown()
{
#ifndef _WIN32
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
#endif
    return true;
}
