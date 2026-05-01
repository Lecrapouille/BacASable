#include "DifferentialDriveMock.h"

namespace kinematics {

DifferentialDrive::DifferentialDrive(Length wheel_radius, Length track_width)
    : wheel_radius_(wheel_radius), track_width_(track_width) {}

WheelVelocities DifferentialDrive::twist_to_wheels(const Twist& twist) const
{
    if (auto* m = DifferentialDriveMock::mock())
        return m->twist_to_wheels(twist);
    return {};
}

Twist DifferentialDrive::wheels_to_twist(const WheelVelocities& wheels) const
{
    if (auto* m = DifferentialDriveMock::mock())
        return m->wheels_to_twist(wheels);
    return {};
}

} // namespace kinematics
