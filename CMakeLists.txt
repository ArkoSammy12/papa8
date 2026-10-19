cmake_minimum_required(VERSION 4.3)
project(papa8 C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        SDL3
        GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
        GIT_TAG main
        EXCLUDE_FROM_ALL
)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/build/libs)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/build/libs)

FetchContent_MakeAvailable(SDL3)

set(SOURCES
        src/main.c
        src/chip8.c
        src/chip8_host.c
)

add_executable(papa8 ${SOURCES})

target_include_directories(papa8 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(papa8 PRIVATE SDL3::SDL3)