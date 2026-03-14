# odometry - Dead-reckoning pose estimation from wheel encoders
add_module_library(odometry
    VERSION 0.1.0
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        kinematics
        mp-units::mp-units
)
