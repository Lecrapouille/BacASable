#pragma once

#include "odometry/WheelOdometry.h"

#include <gmock/gmock.h>

namespace odometry {

struct WheelOdometryMock {
    WheelOdometryMock()  { instance_ = this; }
    ~WheelOdometryMock() { instance_ = nullptr; }

    MOCK_METHOD(void, update, (int delta_left_ticks, int delta_right_ticks));
    MOCK_METHOD(void, reset, (const Pose2D& pose));

    static WheelOdometryMock* mock() { return instance_; }

private:
    static inline WheelOdometryMock* instance_ = nullptr;
};

} // namespace odometry
