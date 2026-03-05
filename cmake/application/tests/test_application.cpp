#include <gtest/gtest.h>
#include "robot_controller/RobotControllerMock.h"

using namespace mp_units::si::unit_symbols;

class ApplicationTest : public ::testing::Test {
protected:
    robot_controller::RobotControllerMock ctrl_mock_;
    robot_controller::RobotController ctrl_{0.05 * m, 0.30 * m, 1000, 0.05 * s};
};

TEST_F(ApplicationTest, StepDelegatesToController) {
    EXPECT_CALL(ctrl_mock_, step(::testing::_)).Times(1);

    ctrl_.step({0.5 * (m / s), 0.0 * (rad / s)});
}

TEST_F(ApplicationTest, PoseReturnsExpectedValue) {
    static const odometry::Pose2D expected{1.0 * m, 2.0 * m, 0.5 * rad};
    EXPECT_CALL(ctrl_mock_, pose())
        .WillOnce(::testing::ReturnRef(expected));

    const auto& p = ctrl_.pose();
    EXPECT_DOUBLE_EQ(p.x.numerical_value_in(m), 1.0);
    EXPECT_DOUBLE_EQ(p.y.numerical_value_in(m), 2.0);
    EXPECT_DOUBLE_EQ(p.theta.numerical_value_in(rad), 0.5);
}

TEST_F(ApplicationTest, ResetPoseDelegatesToController) {
    const odometry::Pose2D new_pose{3.0 * m, 1.0 * m, 0.0 * rad};
    EXPECT_CALL(ctrl_mock_, reset_pose(::testing::_)).Times(1);

    ctrl_.reset_pose(new_pose);
}
