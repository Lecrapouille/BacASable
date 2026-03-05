#include <gtest/gtest.h>
#include "odometry/WheelOdometry.h"

#include <cmath>
#include <stdexcept>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace mp_units;
using namespace mp_units::si::unit_symbols;

namespace {

// Robot: 0.05 m wheel radius, 0.30 m track width, 1000 ticks/rev
// → meters_per_tick = 2π * 0.05 / 1000 ≈ 3.1416e-4 m/tick
class WheelOdometryTest : public ::testing::Test {
protected:
    odometry::WheelOdometry odom{0.05 * m, 0.30 * m, 1000};
};

// ---- Construction ----

TEST(WheelOdometryConstruction, ThrowsOnBadParams) {
    EXPECT_THROW(odometry::WheelOdometry(0.0 * m,  0.30 * m, 1000), std::invalid_argument);
    EXPECT_THROW(odometry::WheelOdometry(0.05 * m, 0.0 * m,  1000), std::invalid_argument);
    EXPECT_THROW(odometry::WheelOdometry(0.05 * m, 0.30 * m, 0),    std::invalid_argument);
}

// ---- Initial state ----

TEST_F(WheelOdometryTest, InitialPoseIsOrigin) {
    EXPECT_DOUBLE_EQ(odom.pose().x.numerical_value_in(m),     0.0);
    EXPECT_DOUBLE_EQ(odom.pose().y.numerical_value_in(m),     0.0);
    EXPECT_DOUBLE_EQ(odom.pose().theta.numerical_value_in(rad), 0.0);
}

// ---- Straight motion ----

TEST_F(WheelOdometryTest, StraightForward) {
    // One full revolution forward on both wheels
    // distance = 2π * 0.05 ≈ 0.3142 m
    odom.update(1000, 1000);
    const double expected_x = 2.0 * M_PI * 0.05;
    EXPECT_NEAR(odom.pose().x.numerical_value_in(m),     expected_x, 1e-9);
    EXPECT_NEAR(odom.pose().y.numerical_value_in(m),     0.0,        1e-9);
    EXPECT_NEAR(odom.pose().theta.numerical_value_in(rad), 0.0,        1e-9);
}

TEST_F(WheelOdometryTest, StraightBackward) {
    odom.update(-1000, -1000);
    const double expected_x = -(2.0 * M_PI * 0.05);
    EXPECT_NEAR(odom.pose().x.numerical_value_in(m), expected_x, 1e-9);
    EXPECT_NEAR(odom.pose().y.numerical_value_in(m), 0.0,        1e-9);
}

// ---- Rotation in place ----

TEST_F(WheelOdometryTest, RotateInPlace90Degrees) {
    // For a 90° CCW rotation: d_theta = π/2
    // d_right - d_left = track_width * π/2 = 0.30 * π/2
    // In ticks: Δ = (0.30 * π/2) / meters_per_tick
    const double meters_per_tick = 2.0 * M_PI * 0.05 / 1000.0;
    const double arc = 0.30 * M_PI / 2.0 / 2.0; // half-track * angle
    const int ticks = static_cast<int>(std::round(arc / meters_per_tick));
    odom.update(-ticks, ticks);
    EXPECT_NEAR(odom.pose().x.numerical_value_in(m),     0.0,        1e-3);
    EXPECT_NEAR(odom.pose().y.numerical_value_in(m),     0.0,        1e-3);
    EXPECT_NEAR(odom.pose().theta.numerical_value_in(rad), M_PI / 2.0, 1e-3);
}

// ---- Heading normalisation ----

TEST_F(WheelOdometryTest, HeadingStaysInPiRange) {
    // Accumulate many small rotations to exceed 2π
    for (int i = 0; i < 100; ++i)
        odom.update(-10, 10);
    EXPECT_GT(odom.pose().theta.numerical_value_in(rad), -M_PI);
    EXPECT_LE(odom.pose().theta.numerical_value_in(rad),  M_PI);
}

// ---- Reset ----

TEST_F(WheelOdometryTest, ResetRestoresPose) {
    odom.update(500, 500);
    odom.reset({1.0 * m, 2.0 * m, 0.5 * rad});
    EXPECT_DOUBLE_EQ(odom.pose().x.numerical_value_in(m),     1.0);
    EXPECT_DOUBLE_EQ(odom.pose().y.numerical_value_in(m),     2.0);
    EXPECT_DOUBLE_EQ(odom.pose().theta.numerical_value_in(rad), 0.5);
}

} // namespace
