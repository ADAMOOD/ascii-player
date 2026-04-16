cmake_minimum_required(VERSION 3.14) # FTXUI vyžaduje o něco novější CMake
project(AsciiPlayer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")

find_package(OpenCV REQUIRED)

# --- Pridani FTXUI z GitHubu ---
include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
  GIT_TAG main
)
FetchContent_MakeAvailable(ftxui)
# -------------------------------

set(SOURCES 
    src/main.cpp 
    src/AsciiEngine.cpp
    src/Tui.cpp
)

add_executable(ASCII-PLAYER ${SOURCES})

target_include_directories(ASCII-PLAYER PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Musime zlinkovat nejen OpenCV, ale i tri moduly z FTXUI
target_link_libraries(ASCII-PLAYER 
    ${OpenCV_LIBS} 
    ftxui::screen 
    ftxui::dom 
    ftxui::component
)