find_package(Qt5 COMPONENTS Widgets Network REQUIRED)
add_executable(app_ui
    src/main.cpp
    src/MainWindow.cpp
    src/UdpClient.cpp
)
target_include_directories(app_ui PRIVATE include)
target_link_libraries(app_ui PRIVATE 
    Qt5::Widgets 
    Qt5::Network 
    backend
)