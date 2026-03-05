#include "kinematics/DifferentialDrive.h"

#include <stdexcept>

namespace kinematics {

// Bring SI unit symbols into scope: m, s, rad, etc.
using namespace mp_units::si::unit_symbols;

DifferentialDrive::DifferentialDrive(Length wheel_radius, Length track_width)
    : wheel_radius_(wheel_radius), track_width_(track_width)
{
    if (wheel_radius.numerical_value_in(m) <= 0.0)
        throw std::invalid_argument("wheel_radius must be positive");
    if (track_width.numerical_value_in(m) <= 0.0)
        throw std::invalid_argument("track_width must be positive");
}

WheelVelocities DifferentialDrive::twist_to_wheels(const Twist& twist) const
{
    // The formula v_wheel = (v_linear ± ω * b/2) / r mixes linear velocity
    // (m/s), angular velocity (rad/s), and length (m). In strict SI/ISQ,
    // rad*m/s and m/s are distinct dimensions, so we work with numerical
    // values in their respective SI units and reconstruct the result.
    const double v   = twist.linear.numerical_value_in(m / s);
    const double w   = twist.angular.numerical_value_in(rad / s);
    const double r   = wheel_radius_.numerical_value_in(m);
    const double b_2 = track_width_.numerical_value_in(m) / 2.0;

    return {
        (v - w * b_2) / r * (rad / s),
        (v + w * b_2) / r * (rad / s),
    };
}

Twist DifferentialDrive::wheels_to_twist(const WheelVelocities& wheels) const
{
    // Same rationale: extract numerical values, compute, then attach SI units.
    const double r    = wheel_radius_.numerical_value_in(m);
    const double b    = track_width_.numerical_value_in(m);
    const double wl   = wheels.left.numerical_value_in(rad / s);
    const double wr   = wheels.right.numerical_value_in(rad / s);

    return {
        r * (wr + wl) / 2.0 * (m / s),
        r * (wr - wl) / b   * (rad / s),
    };
}

} // namespace kinematics
