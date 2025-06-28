cmake_minimum_required(VERSION 3.16)
project(RadarApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED PRESET_NAME)
    set(PRESET_NAME "default" CACHE STRING "Current preset name")
endif()

if(CMAKE_CONFIGURATION_TYPES)
    set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin/${PRESET_NAME}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
    
    foreach(CFG_TYPE IN LISTS CMAKE_CONFIGURATION_TYPES)
        string(TOUPPER ${CFG_TYPE} CFG_TYPE_UPPER)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_TYPE_UPPER} ${OUTPUT_DIR})
    endforeach()
else()
    set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin/${PRESET_NAME}/${CMAKE_BUILD_TYPE}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIR})
endif()

message(STATUS "Output directory set to: ${OUTPUT_DIR}")

option(BUILD_STATIC_WINDOWS "Build a single static exe on Windows" OFF)

if (WIN32 AND BUILD_STATIC_WINDOWS)
    set(Qt5_DIR "" CACHE PATH "Path to your static Qt5Config.cmake")
    set(Qt5_USE_STATIC_LIBS TRUE)
    add_definitions(-DQT_STATIC)
endif()

add_subdirectory(backend)
add_subdirectory(ui)