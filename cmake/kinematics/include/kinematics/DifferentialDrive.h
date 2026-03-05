#ifndef KINEMATICS_DIFFERENTIAL_DRIVE_H
#define KINEMATICS_DIFFERENTIAL_DRIVE_H

#include <mp-units/systems/si.h>

namespace kinematics {

// SI quantity type aliases used throughout the kinematics module.
using Length          = mp_units::quantity<mp_units::si::metre,                              double>;
using LinearVelocity  = mp_units::quantity<mp_units::si::metre   / mp_units::si::second,     double>;
using AngularVelocity = mp_units::quantity<mp_units::si::radian  / mp_units::si::second,     double>;

/// Wheel angular velocities (rad/s) for a differential-drive robot.
struct WheelVelocities {
    AngularVelocity left;   ///< Left wheel angular velocity  (rad/s)
    AngularVelocity right;  ///< Right wheel angular velocity (rad/s)
};

/// Twist command: linear and angular velocity in the robot body frame.
struct Twist {
    LinearVelocity  linear;   ///< Forward velocity (m/s)
    AngularVelocity angular;  ///< Rotation rate    (rad/s)
};

/**
 * Differential-drive kinematics for a two-wheeled robot.
 *
 * Converts between body-frame Twist commands and individual wheel
 * angular velocities. The track width and wheel radius are expressed
 * as SI Length quantities, preventing silent unit mix-ups.
 *
 * Conventions:
 *   - Positive linear  : forward motion
 *   - Positive angular : counter-clockwise rotation (right-hand rule)
 *   - Wheel velocities : positive = forward
 */
class DifferentialDrive {
public:
    /**
     * Construct a DifferentialDrive model.
     *
     * @param wheel_radius  Radius of each wheel, must be > 0.
     * @param track_width   Distance between the two wheel contact points, must be > 0.
     */
    DifferentialDrive(Length wheel_radius, Length track_width);

    /**
     * Convert a Twist command to wheel angular velocities (inverse kinematics).
     *
     * @param twist  Desired body-frame velocity.
     * @return       Required left and right wheel angular velocities (rad/s).
     */
    WheelVelocities twist_to_wheels(const Twist& twist) const;

    /**
     * Convert wheel angular velocities to a body-frame Twist (forward kinematics).
     *
     * @param wheels  Measured left and right wheel angular velocities (rad/s).
     * @return        Resulting body-frame Twist.
     */
    Twist wheels_to_twist(const WheelVelocities& wheels) const;

    Length wheel_radius() const { return wheel_radius_; }
    Length track_width()  const { return track_width_;  }

private:
    Length wheel_radius_;
    Length track_width_;
};

} // namespace kinematics

#endif // KINEMATICS_DIFFERENTIAL_DRIVE_H
