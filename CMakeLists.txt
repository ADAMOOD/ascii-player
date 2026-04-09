cmake_minimum_required(VERSION 3.10)
project(TesterOpenCV)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

add_executable(ASCII-PLAYER src/main.cpp src/AsciiEngine.cpp)

target_link_libraries(ASCII-PLAYER ${OpenCV_LIBS})