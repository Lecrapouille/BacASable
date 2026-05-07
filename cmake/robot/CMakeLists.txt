# robot - kinematics, odometry, and robot_controller (SUBDIRECTORIES layout)

add_module_library(kinematics
    VERSION 0.1.0
    SUBDIRECTORIES kinematics
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        mp-units::mp-units
)

add_module_mock(kinematics
    SUBDIRECTORIES kinematics
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        mp-units::mp-units
    FORCE_INCLUDES
        mocks/kinematics/DifferentialDriveMock.h
)

add_module_library(odometry
    VERSION 0.1.0
    SUBDIRECTORIES odometry
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        kinematics
        mp-units::mp-units
)

add_module_mock(odometry
    SUBDIRECTORIES odometry
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        kinematics
        mp-units::mp-units
    FORCE_INCLUDES
        mocks/odometry/WheelOdometryMock.h
)

add_module_library(robot_controller
    VERSION 0.1.0
    SUBDIRECTORIES robot_controller
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        kinematics odometry
)

add_module_mock(robot_controller
    SUBDIRECTORIES robot_controller
    PCH pch/pch.hpp
    PUBLIC_DEPENDENCIES
        kinematics odometry
    FORCE_INCLUDES
        mocks/robot_controller/RobotControllerMock.h
)
