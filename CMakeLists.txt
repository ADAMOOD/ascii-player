cmake_minimum_required(VERSION 3.10)
project(AsciiPlayer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# -Wall -Wextra: Compiler alerts every suspisious thing that can cause any trouble
# -O3: Highest optimalization
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")

# Vyhledání OpenCV
find_package(OpenCV REQUIRED)

set(SOURCES 
    src/main.cpp 
    src/AsciiEngine.cpp
)


add_executable(ASCII-PLAYER ${SOURCES})

# 3. TVŮJ DOTAZ: Nastavení výchozí složky pro hlavičky
# Tímto říkáme kompilátoru, ať automaticky hledá .h soubory ve složce "include"
target_include_directories(ASCII-PLAYER PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Nalinkování knihoven
target_link_libraries(ASCII-PLAYER ${OpenCV_LIBS})