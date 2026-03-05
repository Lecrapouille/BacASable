#pragma once

#include <gmock/gmock.h>
#include "odometry/WheelOdometry.h"

namespace odometry {

struct WheelOdometryMock {
    WheelOdometryMock()  { current = this; }
    ~WheelOdometryMock() { current = nullptr; }

    MOCK_METHOD(void, update, (int delta_left_ticks, int delta_right_ticks));
    MOCK_METHOD(void, reset, (const Pose2D& pose));

    static inline WheelOdometryMock* current = nullptr;
};

} // namespace odometry
