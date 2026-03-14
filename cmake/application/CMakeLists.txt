# application - Differential-drive robot simulation
add_module_executable(application
    VERSION 1.0.0
    PCH pch/pch.hpp
    DEPENDENCIES      robot_controller
    TEST_DEPENDENCIES robot_controller_mock
)
