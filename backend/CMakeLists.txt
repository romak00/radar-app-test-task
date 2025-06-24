add_library(backend STATIC
    src/sim-engine.cpp
    src/network-server.cpp
    src/models.cpp
)
target_include_directories(backend PUBLIC include)
find_package(Qt5 COMPONENTS Core Network REQUIRED)
target_link_libraries(backend PUBLIC 
    Qt5::Core 
    Qt5::Network
)