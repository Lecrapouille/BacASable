#pragma once

#include <mp-units/systems/si.h>

namespace odometry {

using Length = mp_units::quantity<mp_units::si::metre,  double>;
using Angle  = mp_units::quantity<mp_units::si::radian, double>;

/// 2D robot pose in the world frame.
struct Pose2D {
    Length x;      ///< X position
    Length y;      ///< Y position
    Angle  theta;  ///< Heading angle, counter-clockwise from X axis
};

inline bool operator==(const Pose2D& a, const Pose2D& b) {
    return a.x == b.x && a.y == b.y && a.theta == b.theta;
}

/// Convenience constant: pose at the world-frame origin with zero heading.
inline constexpr Pose2D kOrigin{
    Length{0.0 * mp_units::si::metre},
    Length{0.0 * mp_units::si::metre},
    Angle {0.0 * mp_units::si::radian},
};

} // namespace odometry
