cmake_minimum_required(VERSION 3.16)
project(cpp-ascii-image LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch the 'stb' headers (contains stb_image.h)
FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git

        GIT_TAG master
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(stb)

add_executable(ascii_art src/main.cpp)


target_include_directories(ascii_art PRIVATE ${stb_SOURCE_DIR})