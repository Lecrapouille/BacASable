#pragma once

#include "robot_controller/RobotController.h"

#include <gmock/gmock.h>

namespace robot_controller {

struct RobotControllerMock {
    RobotControllerMock()  { instance_ = this; }
    ~RobotControllerMock() { instance_ = nullptr; }

    MOCK_METHOD(void, step, (const kinematics::Twist& twist));
    MOCK_METHOD(const odometry::Pose2D&, pose, (), (const));
    MOCK_METHOD(void, reset_pose, (const odometry::Pose2D& pose));

    static RobotControllerMock* mock() { return instance_; }

private:
    static inline RobotControllerMock* instance_ = nullptr;
};

} // namespace robot_controller
