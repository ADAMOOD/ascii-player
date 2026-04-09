#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
AsciiEngine engine;
    if (engine.inicializuj("assets/testEarth.mp4", 100)) { 
        engine.prehraj();
    }
    return 0;
}