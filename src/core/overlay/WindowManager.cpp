#include "core/overlay/WindowManager.h"
#include "SDL_thread.h"
#include <SDL2/SDL_ttf.h>
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
    if (TTF_Init() == -1) {
        std::cerr << "Window manager [ERROR] TTF_Init selhalo: " << TTF_GetError() << std::endl;
        return false;
    }

    m_font = TTF_OpenFont("../assets/Terminal.ttf", 24);
    if (m_font == nullptr) {
        std::cerr << "Font couldn't load: " << TTF_GetError() << std::endl;
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
    if (m_font)
        TTF_CloseFont(m_font);
    TTF_Quit();
    SDL_Quit();
}
void WindowManager::renderBuffer(const std::vector<ImageUtils::Pixel>& buffer, int cols, int rows) {

    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    // data extraction
    std::string frameOutput = "";
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            frameOutput += buffer[y * cols + x].symbol;
        }
        frameOutput += "\n";
    }

    SDL_Color white = {255, 255, 255, 255};
    
    SDL_Surface* surfaceMessage = TTF_RenderUTF8_Blended_Wrapped(m_font, frameOutput.c_str(), white, 0);
    
    if (surfaceMessage) {
        SDL_Texture* messageTexture = SDL_CreateTextureFromSurface(m_renderer, surfaceMessage);
        
        SDL_Rect destRect;
        destRect.x = 0;
        destRect.y = 0;
        destRect.w = surfaceMessage->w;
        destRect.h = surfaceMessage->h; 
        
        SDL_RenderCopy(m_renderer, messageTexture, NULL, &destRect);

        SDL_FreeSurface(surfaceMessage);
        SDL_DestroyTexture(messageTexture);
    }

    SDL_RenderPresent(m_renderer);
}