# kinematics - Differential-drive kinematic model (SI units: m/s, rad/s)
add_module_library(kinematics
    VERSION 0.1.0
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        mp-units::mp-units
)
