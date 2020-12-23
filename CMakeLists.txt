cmake_minimum_required(VERSION 3.0)
project(shdy C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pedantic -Wall -Wextra")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only")
set(GLFW_INSTALL OFF CACHE BOOL "GLFW lib only")

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glad)

add_executable(shdy shdy.c shdy.h main.c)

target_include_directories(shdy PRIVATE vendor/stb/include)
target_link_libraries(shdy PRIVATE glfw glad)

install(TARGETS shdy
        CONFIGURATIONS Release
        RUNTIME DESTINATION ~/Apps/shdy/bin/)
