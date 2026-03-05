#ifndef ROBOT_CONTROLLER_H
#define ROBOT_CONTROLLER_H

#include "kinematics/DifferentialDrive.h"
#include "odometry/WheelOdometry.h"

#include <mp-units/systems/si.h>

namespace robot_controller {

using Duration = mp_units::quantity<mp_units::si::second, double>;

/**
 * High-level controller for a differential-drive robot.
 *
 * Combines kinematic conversion and odometry integration into a single
 * control loop step. At each cycle:
 *   1. Converts a Twist command to wheel velocities (kinematics).
 *   2. Simulates encoder ticks from the commanded velocities.
 *   3. Updates the odometry pose estimate.
 *
 * All physical parameters use SI quantity types: lengths in metres,
 * time in seconds.
 */
class RobotController {
public:
    /**
     * Construct a RobotController.
     *
     * @param wheel_radius   Wheel radius.
     * @param track_width    Distance between wheel contact points.
     * @param ticks_per_rev  Encoder resolution (ticks/revolution).
     * @param dt             Control cycle duration.
     */
    RobotController(kinematics::Length wheel_radius,
                    kinematics::Length track_width,
                    int                ticks_per_rev,
                    Duration           dt);

    /**
     * Execute one control cycle.
     *
     * Converts the twist command to wheel velocities, integrates the
     * resulting encoder ticks, and updates the internal pose estimate.
     *
     * @param twist  Desired body-frame velocity for this cycle.
     */
    void step(const kinematics::Twist& twist);

    /// Current estimated pose.
    const odometry::Pose2D& pose() const;

    /// Reset the pose estimate to the given value.
    void reset_pose(const odometry::Pose2D& pose = odometry::kOrigin);

private:
    kinematics::DifferentialDrive drive_;
    odometry::WheelOdometry       odom_;
    double                        ticks_per_meter_;
    double                        dt_s_;             ///< Control cycle (s)
};

} // namespace robot_controller

#endif // ROBOT_CONTROLLER_H
