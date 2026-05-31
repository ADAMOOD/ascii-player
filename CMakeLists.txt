cmake_minimum_required(VERSION 3.14)
project(AsciiPlayer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Global compiler flags for warnings and optimizations
if(MSVC)
    add_compile_definitions(NOMINMAX)
    # Maximum warning level and optimization flags for MSVC
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /O2 /Ot /Oi /fp:fast")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
endif()

# On linux, we need to link against pthread for threading support (used by FTXUI and our own threading)
find_package(OpenCV REQUIRED)

# --- FTXUI ---
if(WIN32)
    # On windows the user is expected to have FTXUI installed and available via CMake's find_package
    find_package(ftxui CONFIG REQUIRED)
else()
    # On linux we can use FetchContent to automatically download and build FTXUI as part of our build process
    include(FetchContent)
    FetchContent_Declare(ftxui
      GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
      GIT_TAG main
    )
    FetchContent_MakeAvailable(ftxui)
endif()

set(SOURCES 
    src/main.cpp 

    src/core/AsciiEngine.cpp
    src/core/ConfigManager.cpp

    src/ui/Tui.cpp
    src/ui/Property.cpp
    src/strategies/ImageUtilits.cpp
    
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

target_include_directories(ASCII-PLAYER PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(ASCII-PLAYER PRIVATE
    ${OpenCV_LIBS} 
    ftxui::screen 
    ftxui::dom 
    ftxui::component
)