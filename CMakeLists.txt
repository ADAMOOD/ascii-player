cmake_minimum_required(VERSION 3.14)
project(AsciiPlayer)


#to allow CGi on apple
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum OS X version" FORCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Global compiler flags for warnings and optimizations
if(MSVC)
    add_compile_definitions(NOMINMAX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /O2 /Ot /Oi /fp:fast")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
endif()

# On linux, we need to link against pthread for threading support (used by FTXUI and our own threading)
find_package(OpenCV REQUIRED)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

# --- FTXUI ---
if(WIN32)
    # On windows the user is expected to have FTXUI installed and available via CMake's find_package
    find_package(ftxui CONFIG REQUIRED)
else()
    # On linux use FetchContent to automatically download and build FTXUI as part of our build process
    include(FetchContent)
    FetchContent_Declare(ftxui
      GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
      GIT_TAG main
    )
    FetchContent_MakeAvailable(ftxui)
endif()

set(SOURCES 

    src/main.cpp 

    src/core/CoreEngine.cpp
    src/core/TerminalEngine.cpp

    src/core/overlay/OverlayEngine.cpp
    src/core/overlay/WindowManager.cpp


    src/core/ConfigManager.cpp
    src/core/BaseFrameProvider.cpp
    src/core/StreamManager.cpp
    src/core/InputHandler.cpp
    src/core/PerformanceMonitor.cpp

    src/core/overlay/screenCaptures/ScreenCaptureFactory.cpp
    src/core/overlay/screenCaptures/MacScreenCapture.mm

    src/ui/Tui.cpp
    src/ui/Property.cpp
    src/ui/DisplayManager.cpp
    src/ui/DebugWindow.cpp

    src/strategies/ImageUtilities.cpp
    src/strategies/StrategiesFactory.cpp
    src/strategies/AbstractRenderStrategy.cpp
    src/strategies/Grayscale/BaseGrayscaleStrategy.cpp
    src/strategies/EdgeDetections/BaseEdgeDetectionStrategy.cpp
    src/strategies/EdgeDetections/SobelEdgeDetectionStrategy.cpp
    src/strategies/EdgeDetections/AdvancedEdgeDetectionStrategy.cpp
    src/strategies/EdgeDetections/ComicEdgeDetectionStrategy.cpp

    src/ui/SettingsScreen.cpp
)

add_executable(ASCII-PLAYER ${SOURCES})

target_include_directories(ASCII-PLAYER PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    /opt/homebrew/include
)
target_link_directories(ASCII-PLAYER PRIVATE /opt/homebrew/lib)

target_link_libraries(ASCII-PLAYER PRIVATE
    ${OpenCV_LIBS}
    ftxui::component
    -lSDL2
    -lSDL2_ttf
)

# Exclusive for Apple screen capture
if(APPLE)
    target_link_libraries(ASCII-PLAYER PRIVATE "-framework CoreGraphics" "-framework CoreFoundation")
endif()