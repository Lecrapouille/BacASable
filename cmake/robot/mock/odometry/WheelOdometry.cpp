#include "odometry/WheelOdometry.h"
#include "odometry/WheelOdometryMock.h"

namespace odometry {

using namespace mp_units::si::unit_symbols;

WheelOdometry::WheelOdometry(Length wheel_radius,
                              Length track_width,
                              int    ticks_per_rev,
                              Pose2D initial_pose)
    : meters_per_tick_(0.0)
    , track_width_m_(track_width.numerical_value_in(m))
    , pose_(initial_pose)
{}

void WheelOdometry::update(int delta_left_ticks, int delta_right_ticks)
{
    if (WheelOdometryMock::current)
        WheelOdometryMock::current->update(delta_left_ticks, delta_right_ticks);
}

void WheelOdometry::reset(const Pose2D& pose)
{
    if (WheelOdometryMock::current)
        WheelOdometryMock::current->reset(pose);
    pose_ = pose;
}

} // namespace odometry
