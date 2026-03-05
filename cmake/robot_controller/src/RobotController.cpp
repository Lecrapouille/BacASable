#include "robot_controller/RobotController.h"

#include <cmath>
#include <numbers>

namespace robot_controller {

using namespace mp_units::si::unit_symbols;

RobotController::RobotController(kinematics::Length wheel_radius,
                                  kinematics::Length track_width,
                                  int                ticks_per_rev,
                                  Duration           dt)
    : drive_(wheel_radius, track_width)
    , odom_(wheel_radius, track_width, ticks_per_rev)
    , ticks_per_meter_(ticks_per_rev / (2.0 * std::numbers::pi * wheel_radius.numerical_value_in(m)))
    , dt_s_(dt.numerical_value_in(s))
{}

void RobotController::step(const kinematics::Twist& twist)
{
    auto wheel_vel = drive_.twist_to_wheels(twist);

    // wheel_vel [rad/s] * wheel_radius [m] * dt [s] = arc length [m]
    // NOTE: In a real system these values come from hardware encoder readings.
    const double r = drive_.wheel_radius().numerical_value_in(m);
    const double arc_left  = wheel_vel.left.numerical_value_in(rad / s)  * r * dt_s_;
    const double arc_right = wheel_vel.right.numerical_value_in(rad / s) * r * dt_s_;

    const int delta_left  = static_cast<int>(std::round(arc_left  * ticks_per_meter_));
    const int delta_right = static_cast<int>(std::round(arc_right * ticks_per_meter_));

    odom_.update(delta_left, delta_right);
}

const odometry::Pose2D& RobotController::pose() const
{
    return odom_.pose();
}

void RobotController::reset_pose(const odometry::Pose2D& pose)
{
    odom_.reset(pose);
}

} // namespace robot_controller
