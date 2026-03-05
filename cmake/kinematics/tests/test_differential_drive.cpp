#include <gtest/gtest.h>
#include "kinematics/DifferentialDrive.h"

#include <cmath>
#include <stdexcept>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace mp_units;
using namespace mp_units::si::unit_symbols;

namespace {

// Robot: 0.05 m wheel radius, 0.30 m track width
class DifferentialDriveTest : public ::testing::Test {
protected:
    kinematics::DifferentialDrive drive{0.05 * m, 0.30 * m};
};

// ---- Construction ----

TEST(DifferentialDriveConstruction, ThrowsOnZeroWheelRadius) {
    EXPECT_THROW(kinematics::DifferentialDrive(0.0 * m, 0.30 * m), std::invalid_argument);
}

TEST(DifferentialDriveConstruction, ThrowsOnNegativeTrackWidth) {
    EXPECT_THROW(kinematics::DifferentialDrive(0.05 * m, -0.1 * m), std::invalid_argument);
}

// ---- Inverse kinematics: twist → wheels ----

TEST_F(DifferentialDriveTest, PureForwardMotion) {
    // 0.5 m/s forward, no rotation → both wheels at same speed
    kinematics::Twist twist{0.5 * m / s, 0.0 * rad / s};
    auto w = drive.twist_to_wheels(twist);
    EXPECT_DOUBLE_EQ(w.left.numerical_value_in(rad / s),  0.5 / 0.05);
    EXPECT_DOUBLE_EQ(w.right.numerical_value_in(rad / s), 0.5 / 0.05);
}

TEST_F(DifferentialDriveTest, PureRotationInPlace) {
    // No translation, rotate at 1 rad/s
    // v_left  = -(0.15 / 0.05) = -3 rad/s
    // v_right = +(0.15 / 0.05) = +3 rad/s
    kinematics::Twist twist{0.0 * m / s, 1.0 * rad / s};
    auto w = drive.twist_to_wheels(twist);
    EXPECT_DOUBLE_EQ(w.left.numerical_value_in(rad / s),  -3.0);
    EXPECT_DOUBLE_EQ(w.right.numerical_value_in(rad / s),  3.0);
}

TEST_F(DifferentialDriveTest, CombinedMotion) {
    kinematics::Twist twist{0.5 * m / s, 1.0 * rad / s};
    auto w = drive.twist_to_wheels(twist);
    // left  = (0.5 - 1.0*0.15) / 0.05 = 0.35/0.05 = 7
    // right = (0.5 + 1.0*0.15) / 0.05 = 0.65/0.05 = 13
    EXPECT_DOUBLE_EQ(w.left.numerical_value_in(rad / s),   7.0);
    EXPECT_DOUBLE_EQ(w.right.numerical_value_in(rad / s), 13.0);
}

// ---- Forward kinematics: wheels → twist ----

TEST_F(DifferentialDriveTest, WheelsToTwistPureForward) {
    // Both wheels at 10 rad/s → linear = 0.05 * 10 = 0.5 m/s, angular = 0
    kinematics::WheelVelocities wheels{10.0 * rad / s, 10.0 * rad / s};
    auto t = drive.wheels_to_twist(wheels);
    EXPECT_DOUBLE_EQ(t.linear.numerical_value_in(m / s),  0.5);
    EXPECT_DOUBLE_EQ(t.angular.numerical_value_in(rad / s), 0.0);
}

TEST_F(DifferentialDriveTest, WheelsToTwistPureRotation) {
    // left = -3, right = +3 → linear = 0, angular = 0.05*(6)/0.30 = 1 rad/s
    kinematics::WheelVelocities wheels{-3.0 * rad / s, 3.0 * rad / s};
    auto t = drive.wheels_to_twist(wheels);
    EXPECT_DOUBLE_EQ(t.linear.numerical_value_in(m / s),  0.0);
    EXPECT_DOUBLE_EQ(t.angular.numerical_value_in(rad / s), 1.0);
}

// ---- Round-trip consistency ----

TEST_F(DifferentialDriveTest, RoundTripTwistToWheelsAndBack) {
    kinematics::Twist original{0.3 * m / s, 0.5 * rad / s};
    auto wheels = drive.twist_to_wheels(original);
    auto recovered = drive.wheels_to_twist(wheels);
    EXPECT_NEAR(recovered.linear.numerical_value_in(m / s),  original.linear.numerical_value_in(m / s),  1e-9);
    EXPECT_NEAR(recovered.angular.numerical_value_in(rad / s), original.angular.numerical_value_in(rad / s), 1e-9);
}

} // namespace
