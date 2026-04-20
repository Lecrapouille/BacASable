#ifndef ODOMETRY_WHEEL_ODOMETRY_H
#define ODOMETRY_WHEEL_ODOMETRY_H

#include "odometry/Pose2D.h"

namespace odometry {

/**
 * Dead-reckoning odometry for a differential-drive robot.
 *
 * Integrates incremental wheel encoder ticks to estimate the robot pose
 * (x, y, θ) in the world frame.
 *
 * Wheel geometry parameters are expressed as SI Length quantities.
 * The integration uses the exact arc formula for better accuracy than
 * the simple Euler approximation when the robot is turning.
 */
class WheelOdometry {
public:
    /**
     * Construct a WheelOdometry estimator.
     *
     * @param wheel_radius   Radius of each wheel, must be > 0.
     * @param track_width    Distance between wheel contact points, must be > 0.
     * @param ticks_per_rev  Encoder ticks per full wheel revolution, must be > 0.
     * @param initial_pose   Starting pose (default: world-frame origin).
     */
    WheelOdometry(Length wheel_radius,
                  Length track_width,
                  int    ticks_per_rev,
                  Pose2D initial_pose = kOrigin);

    /**
     * Update the pose estimate from raw encoder tick increments.
     *
     * Call this at each control cycle with the number of ticks accumulated
     * since the last call.
     *
     * @param delta_left_ticks   Tick increment for the left  wheel (can be negative).
     * @param delta_right_ticks  Tick increment for the right wheel (can be negative).
     */
    void update(int delta_left_ticks, int delta_right_ticks);

    /// Current estimated pose in the world frame.
    const Pose2D& pose() const { return pose_; }

    /// Reset the pose to a given value (e.g. after a localisation correction).
    void reset(const Pose2D& pose = kOrigin);

private:
    double meters_per_tick_;  ///< Arc length per encoder tick (m/tick)
    double track_width_m_;    ///< Track width in metres (cached for inner loop)
    Pose2D pose_;
};

} // namespace odometry

#endif // ODOMETRY_WHEEL_ODOMETRY_H
