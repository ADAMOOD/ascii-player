#include "core/overlay/WindowManager.h"
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <iostream>

bool WindowManager::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Window manager [ERROR] " << SDL_GetError();
        return false;
    }

    m_window = SDL_CreateWindow(
        "ASCII Overlay", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN
    );
    if (!m_window) {
        std::cerr << "Window manager [ERROR] unable to create SDL window";
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer) {
        std::cerr << "Window manager [ERROR] unable to create SDL Renderer";
        return false;
    }
    return true;
}
bool WindowManager::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            return false;
        }
    }
    return true;
}
WindowManager::~WindowManager() {
    if (m_renderer)
        SDL_DestroyRenderer(m_renderer);
    if (m_window)
        SDL_DestroyWindow(m_window);
    SDL_Quit();
}