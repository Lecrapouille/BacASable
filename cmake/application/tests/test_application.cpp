// The mock header MUST be included first so the macro-rename trick fires
// before the real RobotController.h has had a chance to define the class.
// At build time the mock_robot_controller INTERFACE library also force-
// includes this same header on every TU, so the IDE / standalone runs and
// the actual build agree on the visible symbols.
#include "robot_controller/RobotControllerMock.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace mp_units::si::unit_symbols;
using ::testing::_;
using ::testing::Eq;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::InSequence;

// ---------------------------------------------------------------------------
// Test fixture
//
// Constructing a RobotControllerMock activates the singleton (instance_ = this).
// Any subsequent call on a RobotController instance goes through the bridge
// .cpp in mocks/ which forwards to the mock when instance_ is non-null.
// ---------------------------------------------------------------------------
class ApplicationTest : public ::testing::Test {
protected:
    robot_controller::RobotControllerMock mock_;
    robot_controller::RobotController     ctrl_{0.05 * m, 0.30 * m, 1000, 0.05 * s};
};

// ---------------------------------------------------------------------------
// Delegation verification — simple calls
// ---------------------------------------------------------------------------

// step() forwards the Twist value unchanged
TEST_F(ApplicationTest, StepForwardsExactTwist) {
    const kinematics::Twist cmd{0.5 * (m / s), 0.3 * (rad / s)};
    EXPECT_CALL(mock_, step(cmd)).Times(1);
    ctrl_.step(cmd);
}

// step() is called exactly N times, no more, no less
TEST_F(ApplicationTest, MultipleStepsAreCounted) {
    EXPECT_CALL(mock_, step(_)).Times(10);
    for (int i = 0; i < 10; ++i)
        ctrl_.step({0.0 * (m / s), 0.0 * (rad / s)});
}

// step() must not call pose() internally (side-effect isolation)
TEST_F(ApplicationTest, StepDoesNotCallPose) {
    EXPECT_CALL(mock_, step(_)).Times(3);
    EXPECT_CALL(mock_, pose()).Times(0);
    for (int i = 0; i < 3; ++i)
        ctrl_.step({0.5 * (m / s), 0.0 * (rad / s)});
}

// ---------------------------------------------------------------------------
// Return-value verification for pose()
// ---------------------------------------------------------------------------

// pose() returns exactly the value provided by the mock
TEST_F(ApplicationTest, PoseReturnsMockedValue) {
    static const odometry::Pose2D expected{2.5 * m, -1.0 * m, 0.8 * rad};
    EXPECT_CALL(mock_, pose()).WillOnce(ReturnRef(expected));

    const auto& p = ctrl_.pose();
    EXPECT_DOUBLE_EQ(p.x    .numerical_value_in(m),   2.5);
    EXPECT_DOUBLE_EQ(p.y    .numerical_value_in(m),  -1.0);
    EXPECT_DOUBLE_EQ(p.theta.numerical_value_in(rad), 0.8);
}

// pose() can be queried multiple times and always returns the same value
TEST_F(ApplicationTest, PoseIsIdempotent) {
    static const odometry::Pose2D fixed{1.0 * m, 0.0 * m, 0.0 * rad};
    EXPECT_CALL(mock_, pose()).Times(3).WillRepeatedly(ReturnRef(fixed));

    for (int i = 0; i < 3; ++i) {
        const auto& p = ctrl_.pose();
        EXPECT_DOUBLE_EQ(p.x.numerical_value_in(m), 1.0);
    }
}

// ---------------------------------------------------------------------------
// reset_pose() verification
// ---------------------------------------------------------------------------

// reset_pose() forwards the Pose2D value unchanged
TEST_F(ApplicationTest, ResetPoseForwardsExactPose) {
    const odometry::Pose2D target{3.0 * m, 1.5 * m, 1.0 * rad};
    EXPECT_CALL(mock_, reset_pose(target)).Times(1);
    ctrl_.reset_pose(target);
}

// reset_pose() with no argument uses kOrigin (the default declared in the header)
TEST_F(ApplicationTest, ResetWithoutArgUsesOrigin) {
    EXPECT_CALL(mock_, reset_pose(odometry::kOrigin)).Times(1);
    ctrl_.reset_pose();
}

// ---------------------------------------------------------------------------
// Call-order verification
// ---------------------------------------------------------------------------

// A step followed by a pose query must arrive in that order
TEST_F(ApplicationTest, StepThenPoseIsOrdered) {
    static const odometry::Pose2D after{0.5 * m, 0.0 * m, 0.0 * rad};
    {
        InSequence seq;
        EXPECT_CALL(mock_, step(_)).Times(1);
        EXPECT_CALL(mock_, pose()).WillOnce(ReturnRef(after));
    }
    ctrl_.step({0.5 * (m / s), 0.0 * (rad / s)});
    const auto& p = ctrl_.pose();
    EXPECT_DOUBLE_EQ(p.x.numerical_value_in(m), 0.5);
}

// Full scenario: drive → read pose → reset → read pose back at origin
TEST_F(ApplicationTest, DriveReadResetSequence) {
    static const odometry::Pose2D mid{1.0 * m, 0.0 * m, 0.0 * rad};
    {
        InSequence seq;
        EXPECT_CALL(mock_, step(_)).Times(20);
        EXPECT_CALL(mock_, pose()).WillOnce(ReturnRef(mid));
        EXPECT_CALL(mock_, reset_pose(odometry::kOrigin)).Times(1);
        EXPECT_CALL(mock_, pose()).WillOnce(ReturnRef(odometry::kOrigin));
    }

    for (int i = 0; i < 20; ++i)
        ctrl_.step({0.5 * (m / s), 0.0 * (rad / s)});

    const auto& p1 = ctrl_.pose();
    EXPECT_DOUBLE_EQ(p1.x.numerical_value_in(m), 1.0);

    ctrl_.reset_pose();

    const auto& p2 = ctrl_.pose();
    EXPECT_DOUBLE_EQ(p2.x.numerical_value_in(m), 0.0);
}
