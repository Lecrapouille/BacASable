cmake_minimum_required(VERSION 3.25)
project(Qt6DisplayApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Search for Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable Qt features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Define the executable
add_executable(Qt6DisplayApp
    main.cpp
    MainWindow.cpp
    MainWindow.hpp
    MainWindow.ui
    DrawingWidget.cpp
    DrawingWidget.hpp
)

# Link with Qt6 libraries
target_link_libraries(Qt6DisplayApp
    Qt6::Core
    Qt6::Widgets
)

# Configure for Windows (optional)
if(WIN32)
    set_target_properties(Qt6DisplayApp PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()