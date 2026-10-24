#include "strategies/ImageUtilities.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <opencv2/core/mat.hpp>

class WindowManager {
  private:
    SDL_Window* m_window = NULL;
    SDL_Renderer* m_renderer = NULL;
    TTF_Font* m_font = nullptr;
    std::vector<ImageUtils::Pixel> m_frameBuffer;

  public:
    std::vector<ImageUtils::Pixel>& getBuffer();
    
    void calculateGridSize(int& cols, int& rows);

    void getSize(int* width_out, int* height_out) {
        return SDL_GetWindowSize(m_window, width_out, height_out);
    }
    ~WindowManager();
    bool init();
    bool pollEvents();
    void renderBuffer(int cols, int rows);
};