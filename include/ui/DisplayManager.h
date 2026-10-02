#pragma once 
#include <vector>
#include "strategies/ImageUtilits.h"
#include "ui/Property.h"

class DisplayManager{

    private:
    std::vector<ImageUtils::Pixel> m_frameBuffer; ///< 1D array representing the 2D terminal screen.
    int m_width;                             ///< Current terminal width in characters.
    int m_height;                            ///< Current terminal height in characters.
    double m_aspectRatio;                    ///< Original aspect ratio of the video source.

    public:

    std::vector<ImageUtils::Pixel>& getBuffer();
    int getWidth() const;
    int getHeight() const;
        /**
     * @brief Polls the OS for the current terminal dimensions and adjusts the internal frame buffer.
     */
    void updateTerminalSize();
        /**
     * @brief Renders the current frame buffer to the terminal, applying ANSI color codes if enabled.
     *  Also handles the display of the HUD with active properties and highlights the selected one.
     * 
     */
    void renderBuffer(bool useColor,bool use8Bit,int tolerance);
        /**
     * @brief Renders the heads-up display at the bottom of the terminal, showing active properties of the current strategy.
     * 
     */
    void renderHUD(std::vector<Property> active_properties, int selected_property_index, int menu_start_index);

    void showDebugWindow(const cv::Mat& debugFrame);

    void init(double original_aspectRatio);
};