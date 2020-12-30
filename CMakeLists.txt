cmake_minimum_required(VERSION 3.0)
project(shdy CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Wextra")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only")
set(GLFW_INSTALL OFF CACHE BOOL "GLFW lib only")

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glad)

add_executable(shdy shdy.cpp shdy.h main.cpp)

target_include_directories(shdy PRIVATE vendor/stb/include)
target_link_libraries(shdy PRIVATE glfw glad)

install(TARGETS shdy
        CONFIGURATIONS Release
        RUNTIME DESTINATION ~/Apps/shdy/bin/)
