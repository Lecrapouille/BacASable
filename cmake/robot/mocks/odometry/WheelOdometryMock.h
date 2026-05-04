#pragma once

// Header-only mock for odometry::WheelOdometry — see DifferentialDriveMock.h
// for a detailed explanation of the pattern.

#define WheelOdometry _WheelOdometry_Real
#include "odometry/WheelOdometry.h"
#undef WheelOdometry

#include <gmock/gmock.h>

namespace odometry {

class WheelOdometry {
public:
    struct Mock {
        Mock()  { instance_ = this; }
        ~Mock() { instance_ = nullptr; }

        MOCK_METHOD(void,          update, (int delta_left_ticks, int delta_right_ticks));
        MOCK_METHOD(const Pose2D&, pose,   (), (const));
        MOCK_METHOD(void,          reset,  (const Pose2D& pose));
    };

    WheelOdometry(Length, Length, int, Pose2D = kOrigin) {}

    void update(int delta_left_ticks, int delta_right_ticks) {
        if (instance_) instance_->update(delta_left_ticks, delta_right_ticks);
    }
    const Pose2D& pose() const {
        return instance_ ? instance_->pose() : kOrigin;
    }
    void reset(const Pose2D& p = kOrigin) {
        if (instance_) instance_->reset(p);
    }

private:
    static inline Mock* instance_ = nullptr;
};

using WheelOdometryMock = WheelOdometry::Mock;

} // namespace odometry
