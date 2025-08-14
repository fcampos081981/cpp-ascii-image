# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(cpp-ascii-image LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG        master
)
FetchContent_MakeAvailable(stb)

add_executable(ascii_art
        src/main.cpp
        src/stb_image_impl.cpp
        src/main.cpp

)

# Make stb headers available
target_include_directories(ascii_art PRIVATE ${stb_SOURCE_DIR})