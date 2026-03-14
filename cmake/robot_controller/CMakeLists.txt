# robot_controller - Differential-drive robot control logic
add_module_library(robot_controller
    VERSION 0.1.0
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES kinematics odometry
)
