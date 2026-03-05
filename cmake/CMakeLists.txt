cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)             # Language standard to use

project(RoboticsTemplate
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "CMake C++ template: differential-drive robot"
)

include(cmake/ProjectBootstrap.cmake)

add_subdirectory(kinematics)       # Differential-drive kinematic model (library)
add_subdirectory(odometry)         # Dead-reckoning pose estimation (library)
add_subdirectory(robot_controller) # Robot control logic (library)
add_subdirectory(application)      # Main application (executable)
