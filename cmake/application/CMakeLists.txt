# application - Differential-drive robot simulation
add_module_executable(application
    PCH pch/pch.hpp
    DEPENDENCIES      robot_controller
    TEST_DEPENDENCIES robot_controller_mock
)
