#include <SDL2/SDL.h>
#include <opencv2/core/mat.hpp>
#include <SDL2/SDL_ttf.h>
#include "strategies/ImageUtilities.h"

class WindowManager {
  private:
    SDL_Window *m_window = NULL;
    SDL_Renderer *m_renderer = NULL;
    TTF_Font *m_font = nullptr;

  public:
    void getSize(int *width_out, int *height_out) {
        return SDL_GetWindowSize(m_window, width_out, height_out);
    };
    WindowManager() = default;
    ~WindowManager();
    bool init();
    bool pollEvents();
    void renderBuffer(const std::vector<ImageUtils::Pixel>& buffer, int cols, int rows);
};