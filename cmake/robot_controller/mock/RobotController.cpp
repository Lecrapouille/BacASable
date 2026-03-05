#include "robot_controller/RobotController.h"
#include "robot_controller/RobotControllerMock.h"

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
    if (RobotControllerMock::current) {
        RobotControllerMock::current->step(twist);
        return;
    }
}

const odometry::Pose2D& RobotController::pose() const
{
    if (RobotControllerMock::current)
        return RobotControllerMock::current->pose();
    return odom_.pose();
}

void RobotController::reset_pose(const odometry::Pose2D& pose)
{
    if (RobotControllerMock::current) {
        RobotControllerMock::current->reset_pose(pose);
        return;
    }
    odom_.reset(pose);
}

} // namespace robot_controller
