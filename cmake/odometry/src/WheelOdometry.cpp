#include "odometry/WheelOdometry.h"

#include <cmath>
#include <numbers>
#include <stdexcept>

namespace odometry {

using namespace mp_units::si::unit_symbols;

WheelOdometry::WheelOdometry(Length wheel_radius,
                              Length track_width,
                              int    ticks_per_rev,
                              Pose2D initial_pose)
    : pose_(initial_pose)
{
    const double r = wheel_radius.numerical_value_in(m);
    const double b = track_width.numerical_value_in(m);

    if (r <= 0.0)
        throw std::invalid_argument("wheel_radius must be positive");
    if (b <= 0.0)
        throw std::invalid_argument("track_width must be positive");
    if (ticks_per_rev <= 0)
        throw std::invalid_argument("ticks_per_rev must be positive");

    meters_per_tick_ = (2.0 * std::numbers::pi * r) / ticks_per_rev;
    track_width_m_   = b;
}

void WheelOdometry::update(int delta_left_ticks, int delta_right_ticks)
{
    const double d_left  = delta_left_ticks  * meters_per_tick_;
    const double d_right = delta_right_ticks * meters_per_tick_;

    const double d_center = (d_right + d_left) / 2.0;
    // d_theta [rad] = arc difference / track width  (m/m, dimensionless ratio
    // interpreted as radians by definition of the radian)
    const double d_theta  = (d_right - d_left) / track_width_m_;

    const double theta = pose_.theta.numerical_value_in(rad);

    // Exact arc integration: avoids the small-angle approximation
    if (std::abs(d_theta) < 1e-10) {
        // Straight-line motion
        pose_.x += d_center * std::cos(theta) * m;
        pose_.y += d_center * std::sin(theta) * m;
    } else {
        // Arc motion: integrate along the circular arc
        const double radius = d_center / d_theta;
        pose_.x += radius * (std::sin(theta + d_theta) - std::sin(theta)) * m;
        pose_.y += radius * (std::cos(theta) - std::cos(theta + d_theta)) * m;
    }

    pose_.theta += d_theta * rad;

    // Normalise heading to (-π, π]
    const double pi = std::numbers::pi;
    double th = pose_.theta.numerical_value_in(rad);
    while (th >  pi) th -= 2.0 * pi;
    while (th <= -pi) th += 2.0 * pi;
    pose_.theta = th * rad;
}

void WheelOdometry::reset(const Pose2D& pose)
{
    pose_ = pose;
}

} // namespace odometry
