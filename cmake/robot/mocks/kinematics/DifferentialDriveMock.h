#pragma once

#include "kinematics/DifferentialDrive.h"

#include <gmock/gmock.h>

namespace kinematics {

struct DifferentialDriveMock {
    DifferentialDriveMock()  { instance_ = this; }
    ~DifferentialDriveMock() { instance_ = nullptr; }

    MOCK_METHOD(WheelVelocities, twist_to_wheels, (const Twist&), (const));
    MOCK_METHOD(Twist, wheels_to_twist, (const WheelVelocities&), (const));

    static DifferentialDriveMock* mock() { return instance_; }

private:
    static inline DifferentialDriveMock* instance_ = nullptr;
};

} // namespace kinematics
