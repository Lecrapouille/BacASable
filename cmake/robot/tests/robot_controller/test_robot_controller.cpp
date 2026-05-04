// =============================================================================
// Unit test for robot_controller::RobotController
//
// kinematics::DifferentialDrive and odometry::WheelOdometry are header-only
// mocked thanks to the mock_kinematics / mock_odometry INTERFACE libraries
// declared in robot/CMakeLists.txt (TEST_DEPENDENCIES). Each *Mock.h header is
// force-included on every TU of this test target via -include, so even the
// translation unit that compiles RobotController.cpp uses the mocked classes.
//
// This is exactly the diamond scenario that breaks the previous .cpp-bridge
// approach: robot_controller depends PUBLICLY on kinematics + odometry, so a
// link-time mock for those would coexist with the real implementations and
// produce duplicate symbols (cf. nm output before the refactor).
//
// The explicit mock includes below are technically redundant (force-include
// handles it on the command line) but they MUST appear before any include
// that pulls the real header in, so that an IDE / standalone preprocessor run
// without -include flags still sees the mock first.
// =============================================================================
#include "kinematics/DifferentialDriveMock.h"
#include "odometry/WheelOdometryMock.h"

#include <gtest/gtest.h>
#include "robot_controller/RobotController.h"

using namespace mp_units::si::unit_symbols;
using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace {

class RobotControllerTest : public ::testing::Test {
protected:
    kinematics::DifferentialDriveMock drive_mock_;
    odometry::WheelOdometryMock       odom_mock_;
    robot_controller::RobotController ctrl{0.05 * m, 0.30 * m, 1000, 0.05 * s};
};

// step() converts twist → wheels via DifferentialDrive, then integrates ticks
// into the odometry. We verify the delegation chain.
TEST_F(RobotControllerTest, StepDelegatesToDriveAndOdometry) {
    using namespace kinematics;
    EXPECT_CALL(drive_mock_, twist_to_wheels(Twist{0.5 * (m/s), 0.0 * (rad/s)}))
        .WillOnce(Return(WheelVelocities{10.0 * (rad/s), 10.0 * (rad/s)}));
    EXPECT_CALL(drive_mock_, wheel_radius())
        .WillOnce(Return(0.05 * m));
    EXPECT_CALL(odom_mock_, update(_, _)).Times(1);

    ctrl.step({0.5 * (m/s), 0.0 * (rad/s)});
}

// pose() forwards directly to odometry::WheelOdometry::pose().
TEST_F(RobotControllerTest, PoseForwardsToOdometry) {
    static const odometry::Pose2D expected{1.0 * m, 2.0 * m, 0.5 * rad};
    EXPECT_CALL(odom_mock_, pose()).WillOnce(ReturnRef(expected));

    const auto& p = ctrl.pose();
    EXPECT_DOUBLE_EQ(p.x    .numerical_value_in(m),   1.0);
    EXPECT_DOUBLE_EQ(p.y    .numerical_value_in(m),   2.0);
    EXPECT_DOUBLE_EQ(p.theta.numerical_value_in(rad), 0.5);
}

// reset_pose() forwards to odometry::WheelOdometry::reset().
TEST_F(RobotControllerTest, ResetPoseForwardsToOdometry) {
    const odometry::Pose2D target{3.0 * m, 1.5 * m, 1.0 * rad};
    EXPECT_CALL(odom_mock_, reset(target)).Times(1);

    ctrl.reset_pose(target);
}

// reset_pose() default argument resolves to odometry::kOrigin (defined in the
// real header, still visible because the mock includes the real header via
// the macro-rename trick).
TEST_F(RobotControllerTest, ResetPoseDefaultsToOrigin) {
    EXPECT_CALL(odom_mock_, reset(odometry::kOrigin)).Times(1);

    ctrl.reset_pose();
}

} // namespace
