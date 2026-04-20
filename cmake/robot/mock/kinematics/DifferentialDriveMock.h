#pragma once

#include <gmock/gmock.h>
#include "kinematics/DifferentialDrive.h"

namespace kinematics {

struct DifferentialDriveMock {
    DifferentialDriveMock()  { current = this; }
    ~DifferentialDriveMock() { current = nullptr; }

    MOCK_METHOD(WheelVelocities, twist_to_wheels, (const Twist&), (const));
    MOCK_METHOD(Twist, wheels_to_twist, (const WheelVelocities&), (const));

    static inline DifferentialDriveMock* current = nullptr;
};

} // namespace kinematics
