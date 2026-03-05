#include <gtest/gtest.h>
#include "robot_controller/RobotController.h"

using namespace mp_units::si::unit_symbols;

namespace {

// Robot: 0.05 m wheel radius, 0.30 m track width, 1000 ticks/rev, 0.05 s cycle
class RobotControllerTest : public ::testing::Test {
protected:
    robot_controller::RobotController ctrl{0.05 * m, 0.30 * m, 1000, 0.05 * s};
};

TEST_F(RobotControllerTest, InitialPoseIsOrigin) {
    EXPECT_DOUBLE_EQ(ctrl.pose().x.numerical_value_in(m),     0.0);
    EXPECT_DOUBLE_EQ(ctrl.pose().y.numerical_value_in(m),     0.0);
    EXPECT_DOUBLE_EQ(ctrl.pose().theta.numerical_value_in(rad), 0.0);
}

TEST_F(RobotControllerTest, StraightMotionMovesAlongX) {
    // Drive forward at 0.5 m/s for 2 s (40 steps of 0.05 s)
    for (int i = 0; i < 40; ++i)
        ctrl.step({0.5 * (m / s), 0.0 * (rad / s)});

    // Expected: ~1.0 m forward, no lateral drift, no rotation
    EXPECT_NEAR(ctrl.pose().x.numerical_value_in(m),       1.0, 0.05);  // 5 cm tolerance (encoder rounding)
    EXPECT_NEAR(ctrl.pose().y.numerical_value_in(m),       0.0, 0.01);
    EXPECT_NEAR(ctrl.pose().theta.numerical_value_in(rad), 0.0, 0.01);
}

TEST_F(RobotControllerTest, ZeroTwistKeepsPoseStill) {
    for (int i = 0; i < 20; ++i)
        ctrl.step({0.0 * (m / s), 0.0 * (rad / s)});

    EXPECT_DOUBLE_EQ(ctrl.pose().x.numerical_value_in(m),       0.0);
    EXPECT_DOUBLE_EQ(ctrl.pose().y.numerical_value_in(m),       0.0);
    EXPECT_DOUBLE_EQ(ctrl.pose().theta.numerical_value_in(rad), 0.0);
}

TEST_F(RobotControllerTest, ResetRestoresPose) {
    for (int i = 0; i < 10; ++i)
        ctrl.step({0.5 * (m / s), 0.0 * (rad / s)});

    ctrl.reset_pose({3.0 * m, 1.0 * m, 0.5 * rad});
    EXPECT_DOUBLE_EQ(ctrl.pose().x.numerical_value_in(m),       3.0);
    EXPECT_DOUBLE_EQ(ctrl.pose().y.numerical_value_in(m),       1.0);
    EXPECT_DOUBLE_EQ(ctrl.pose().theta.numerical_value_in(rad), 0.5);
}

} // namespace
