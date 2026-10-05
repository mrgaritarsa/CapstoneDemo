cmake_minimum_required(VERSION 3.16)
project(keydemo C)

# Link SDL statically so there's no DLL/.so to copy next to the executable
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

add_executable(keydemo main.c)
target_link_libraries(keydemo PRIVATE SDL3::SDL3)