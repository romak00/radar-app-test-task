cmake_minimum_required(VERSION 3.16)
project(RadarApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_STATIC_WINDOWS "Build a single static exe on Windows" OFF)

if (WIN32 AND BUILD_STATIC_WINDOWS)
    set(Qt5_DIR "" CACHE PATH "Path to your static Qt5Config.cmake")
    set(Qt5_USE_STATIC_LIBS TRUE)
    add_definitions(-DQT_STATIC)
endif()

add_subdirectory(backend)
add_subdirectory(ui)