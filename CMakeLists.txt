cmake_minimum_required(VERSION 3.14)
project(AsciiPlayer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Global compiler flags for warnings and build types
if(MSVC)
    add_compile_definitions(NOMINMAX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /O2 /Ot /Oi /fp:fast")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

if(APPLE)

else()
    find_package(PkgConfig REQUIRED)
endif()

# --- FTXUI ---
if(WIN32)
    find_package(ftxui CONFIG REQUIRED)
else()
    include(FetchContent)
    FetchContent_Declare(ftxui
            GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
            GIT_TAG main
    )
    FetchContent_MakeAvailable(ftxui)
endif()

# Automatically collect all .cpp and .h files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/*.h")

add_executable(ASCII-PLAYER ${SOURCES} ${HEADERS})

target_include_directories(ASCII-PLAYER PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
        ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(ASCII-PLAYER PRIVATE
        ${OpenCV_LIBS}
        ftxui::component
        Threads::Threads
)

# macOS Homebrew paths
if(APPLE)
    target_include_directories(ASCII-PLAYER PRIVATE /opt/homebrew/include)
    target_link_directories(ASCII-PLAYER PRIVATE /opt/homebrew/lib)
endif()

# Jetson TX2 (Linux ARM64 + TensorRT + CUDA)
if(UNIX AND NOT APPLE)
    find_package(CUDAToolkit REQUIRED)
    target_link_libraries(ASCII-PLAYER PRIVATE nvinfer CUDA::cudart)
    target_compile_definitions(ASCII-PLAYER PRIVATE USE_TENSORRT)
endif()