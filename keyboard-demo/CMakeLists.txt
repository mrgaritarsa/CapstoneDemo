cmake_minimum_required(VERSION 3.16)
project(KeyboardDemo C)

# We only want the GLFW library itself, not its docs, tests, or examples
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

find_package(OpenGL REQUIRED)

add_executable(keyboard_demo main.c)
target_link_libraries(keyboard_demo PRIVATE glfw OpenGL::GL)