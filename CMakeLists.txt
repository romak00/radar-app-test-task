cmake_minimum_required(VERSION 3.16)
project(RadarApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_STATIC_QT "Link Qt statically (Windows only)" OFF)

if(NOT DEFINED PRESET_NAME)
    set(PRESET_NAME "default" CACHE STRING "Current preset name")
endif()

if (BUILD_STATIC_QT)
    message(STATUS "Configuring STATIC Qt integration")
    if (NOT Qt5_DIR)
        message(FATAL_ERROR "BUILD_STATIC_QT=ON but Qt5_DIR is not set")
    endif()
    set(Qt5_USE_STATIC_LIBS TRUE)
    add_definitions(-DQT_STATIC)
else()
    message(STATUS "Configuring DYNAMIC Qt integration")
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

add_subdirectory(backend)
add_subdirectory(ui)