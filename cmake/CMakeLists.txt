cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Install to local install/ directory")

project(RoboticsTemplate
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "CMake C++ template: differential-drive robot"
)

include(cmake/ProjectBootstrap.cmake)

find_conan_package(mp-units REQUIRED)

add_subdirectory(robot)          # Kinematics, odometry, robot_controller (libraries)
add_subdirectory(application)    # Main application (executable)
