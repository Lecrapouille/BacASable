#pragma once

#include <gmock/gmock.h>
#include "robot_controller/RobotController.h"

namespace robot_controller {

struct RobotControllerMock {
    RobotControllerMock()  { current = this; }
    ~RobotControllerMock() { current = nullptr; }

    MOCK_METHOD(void, step, (const kinematics::Twist& twist));
    MOCK_METHOD(const odometry::Pose2D&, pose, (), (const));
    MOCK_METHOD(void, reset_pose, (const odometry::Pose2D& pose));

    static inline RobotControllerMock* current = nullptr;
};

} // namespace robot_controller
