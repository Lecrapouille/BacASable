# kinematics - Differential-drive kinematic model (SI units: m/s, rad/s)
add_module_library(kinematics
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        mp-units::mp-units
)
