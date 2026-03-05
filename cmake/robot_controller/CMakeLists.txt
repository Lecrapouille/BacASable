# robot_controller - Differential-drive robot control logic
add_module_library(robot_controller
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES kinematics odometry
)
