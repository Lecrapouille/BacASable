#pragma once

// Header-only mock for robot_controller::RobotController — see kinematics/
// DifferentialDriveMock.h for a detailed explanation of the pattern.

#define RobotController _RobotController_Real
#include "robot_controller/RobotController.h"
#undef RobotController

#include <gmock/gmock.h>

namespace robot_controller {

class RobotController {
public:
    struct Mock {
        Mock()  { instance_ = this; }
        ~Mock() { instance_ = nullptr; }

        MOCK_METHOD(void,                    step,        (const kinematics::Twist& twist));
        MOCK_METHOD(const odometry::Pose2D&, pose,        (), (const));
        MOCK_METHOD(void,                    reset_pose,  (const odometry::Pose2D& pose));
    };

    RobotController(kinematics::Length, kinematics::Length, int, Duration) {}

    void step(const kinematics::Twist& twist) {
        if (instance_) instance_->step(twist);
    }
    const odometry::Pose2D& pose() const {
        return instance_ ? instance_->pose() : odometry::kOrigin;
    }
    void reset_pose(const odometry::Pose2D& p = odometry::kOrigin) {
        if (instance_) instance_->reset_pose(p);
    }

private:
    static inline Mock* instance_ = nullptr;
};

using RobotControllerMock = RobotController::Mock;

} // namespace robot_controller
