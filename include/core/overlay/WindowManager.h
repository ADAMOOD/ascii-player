#include <SDL2/SDL.h>

class WindowManager {
  private:
    SDL_Window *m_window = NULL;
    SDL_Renderer *m_renderer = NULL;

  public:
    void getSize(int *width_out, int *height_out) {
        return SDL_GetWindowSize(m_window, width_out, height_out);
    };
    WindowManager() = default;
    ~WindowManager();
    bool init();
    bool pollEvents();

};