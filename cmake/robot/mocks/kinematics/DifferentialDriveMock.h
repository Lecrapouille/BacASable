#pragma once

// =============================================================================
// Header-only mock for kinematics::DifferentialDrive
//
// Pattern (cf. Cpp/UnitTests/Mock2/test/MockPublisher.h):
//   1. The real header is included with the class renamed via a macro, so all
//      dependent types (Length, Twist, WheelVelocities, ...) are visible AND
//      tracked: any change to those types breaks the mock at compile time.
//   2. A drop-in replacement class DifferentialDrive is then declared in the
//      same namespace. It exposes the same public API and forwards each call
//      to a singleton gmock instance via RAII auto-registration.
//
// Compared to the previous mock_<lib> static library, no symbol of the real
// class is defined here: the mock substitutes the class at compile time, not
// at link time. There is therefore no risk of ODR / multiple-definition issues
// when a test transitively links the real library AND a mock for one of its
// dependencies.
//
// IMPORTANT: this header MUST be force-included (via -include ...) before any
// other source of the test target, so that an #include of the real header
// coming from a consumer source is intercepted (the real header is already
// marked as included via #pragma once after the macro trick below).
// =============================================================================

#define DifferentialDrive _DifferentialDrive_Real
#include "kinematics/DifferentialDrive.h"
#undef DifferentialDrive

#include <gmock/gmock.h>

namespace kinematics {

class DifferentialDrive {
public:
    struct Mock {
        Mock()  { instance_ = this; }
        ~Mock() { instance_ = nullptr; }

        MOCK_METHOD(WheelVelocities, twist_to_wheels, (const Twist&), (const));
        MOCK_METHOD(Twist,           wheels_to_twist, (const WheelVelocities&), (const));
        MOCK_METHOD(Length,          wheel_radius,    (), (const));
        MOCK_METHOD(Length,          track_width,     (), (const));
    };

    DifferentialDrive(Length, Length) {}

    WheelVelocities twist_to_wheels(const Twist& t) const {
        return instance_ ? instance_->twist_to_wheels(t) : WheelVelocities{};
    }
    Twist wheels_to_twist(const WheelVelocities& w) const {
        return instance_ ? instance_->wheels_to_twist(w) : Twist{};
    }
    Length wheel_radius() const {
        return instance_ ? instance_->wheel_radius() : Length{};
    }
    Length track_width() const {
        return instance_ ? instance_->track_width() : Length{};
    }

private:
    static inline Mock* instance_ = nullptr;
};

using DifferentialDriveMock = DifferentialDrive::Mock;

} // namespace kinematics
