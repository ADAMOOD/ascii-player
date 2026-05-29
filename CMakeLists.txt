cmake_minimum_required(VERSION 3.14)
project(AsciiPlayer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Globální ošetření kompilátorů (Windows MSVC vs Linux GCC)
if(MSVC)
    add_compile_definitions(NOMINMAX)
    # Přidána maximální optimalizace rychlosti a agresivní matematika pro OpenCV
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /O2 /Ot /Oi /fp:fast")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
endif()

# OpenCV se najde všude (na Win přes vcpkg, na Linuxu přes apt-get)
find_package(OpenCV REQUIRED)

# --- CHYTRÁ VÝHYBKA PRO FTXUI ---
if(WIN32)
    # Na Windows už ho máme zkompilovaný z vcpkg
    find_package(ftxui CONFIG REQUIRED)
else()
    # Na Linuxu si ho CMake stáhne sám z GitHubu, přesně jak jsi zvyklý
    include(FetchContent)
    FetchContent_Declare(ftxui
      GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
      GIT_TAG main
    )
    FetchContent_MakeAvailable(ftxui)
endif()
# --------------------------------

set(SOURCES 
    src/main.cpp 
    src/core/AsciiEngine.cpp
    src/core/ConfigManager.cpp
    src/ui/Tui.cpp
    src/ui/Property.cpp
    src/strategies/StrategiesFactory.cpp
    src/strategies/ImageUtilits.cpp
    src/strategies/BaseGrayscaleStrategy.cpp
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