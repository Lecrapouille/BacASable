# robot - kinematics, odometry, and robot_controller (SUBDIRECTORIES layout)

library(kinematics
    VERSION 0.1.0
    SUBDIRECTORIES kinematics
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        mp-units::mp-units
)

library(odometry
    VERSION 0.1.0
    SUBDIRECTORIES odometry
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        kinematics
        mp-units::mp-units
)

library(robot_controller
    VERSION 0.1.0
    SUBDIRECTORIES robot_controller
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        kinematics odometry
)
