cmake_minimum_required(VERSION 3.16)
project(RadarUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets Network Core OpenGL Gui REQUIRED)

set(COMMON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/common/include)
set(COMMON_SRC ${CMAKE_SOURCE_DIR}/common/src)
set(EIGEN_ROOT ${CMAKE_SOURCE_DIR}/thirdparty/eigen/eigen-master)

if(WIN32)
    add_executable(radar_ui WIN32
        src/main.cpp
        src/window.cpp
        src/network-client.cpp
        src/radar-widget.cpp
        ${COMMON_SRC}/target.cpp
        include/window.h
        include/network-client.h
        include/radar-widget.h
    )
else()
    add_executable(radar_ui
        src/main.cpp
        src/window.cpp
        src/network-client.cpp
        src/radar-widget.cpp
        ${COMMON_SRC}/target.cpp
        include/window.h
        include/network-client.h
        include/radar-widget.h
    )
endif()

target_include_directories(radar_ui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${COMMON_INCLUDE_DIR}
    ${EIGEN_ROOT}
)

target_link_libraries(radar_ui PRIVATE
    Qt5::Widgets
    Qt5::Network
    Qt5::Core
    Qt5::OpenGL
    Qt5::Gui
)

if (WIN32 AND BUILD_STATIC_WINDOWS)
    target_compile_definitions(radar_ui PRIVATE QT_STATIC)
endif()

set_target_properties(radar_ui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)