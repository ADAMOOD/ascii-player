#include <opencv2/opencv.hpp>
#include <iostream>
#include "AsciiEngine.h"

using namespace std;
int main() {
    AsciiEngine engine;

    cout << "--- Spoustim test AsciiEngine ---" << endl;
    
    // Zkusíme inicializovat engine s testovacím videem z minula
    // Cílová šířka v terminálu bude 100 znaků
    if (engine.init("../assets/testEarth.mp4", 100)) {
        cout << "[USPECH] Video bylo nacteno." << endl;
        cout << "[USPECH] Pamet pro ASCII buffer uspesne alokovana!" << endl;
        engine.play();
        
        // Zde budeme v budoucnu volat engine.play()
    } else {
        cerr << "[CHYBA] Nepodarilo se inicializovat engine." << endl;
    }

    return 0;
}