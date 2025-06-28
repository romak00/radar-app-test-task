cmake_minimum_required(VERSION 3.16)
project(RadarBackend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMMON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/common/include)
set(COMMON_SRC_DIR ${CMAKE_SOURCE_DIR}/common/src)

set(BE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(BE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SERVER_SOURCES
    ${BE_SRC_DIR}/main.cpp
    ${BE_SRC_DIR}/network-server.cpp
    ${BE_SRC_DIR}/sim-engine.cpp
    ${COMMON_SRC_DIR}/target.cpp
)

if(WIN32)
    add_executable(radar_server WIN32 ${SERVER_SOURCES})
    
    set_target_properties(radar_server PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
else()
    add_executable(radar_server ${SERVER_SOURCES})
endif()


target_include_directories(radar_server PRIVATE
    ${BE_INCLUDE_DIR}
    ${COMMON_INCLUDE_DIR}
)

add_library(asio INTERFACE)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
target_include_directories(asio INTERFACE
    ${CMAKE_SOURCE_DIR}/thirdparty/asio/include
)

add_library(eigen INTERFACE)
target_include_directories(eigen INTERFACE
    ${CMAKE_SOURCE_DIR}/thirdparty/eigen/eigen-master
)

target_link_libraries(radar_server PRIVATE
    asio
    eigen
)

if (UNIX)
    target_link_libraries(radar_server PRIVATE pthread)
endif()

if(WIN32)
    target_compile_definitions(radar_server PRIVATE 
        _WIN32_WINNT=0x0A00
        ASIO_STANDALONE
    )
endif()

set_target_properties(radar_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)