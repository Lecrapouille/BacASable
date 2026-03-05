#include "robot_controller/RobotController.h"

#include <iomanip>
#include <iostream>
#include <numbers>

int main()
{
    using namespace mp_units::si::unit_symbols;

    // Robot geometry — SI units enforced at compile time
    constexpr auto wheel_radius  = 0.05 * m;
    constexpr auto track_width   = 0.30 * m;
    constexpr int  ticks_per_rev = 1000;
    constexpr auto dt            = 0.05 * s;   // 20 Hz control loop

    robot_controller::RobotController controller(
        wheel_radius, track_width, ticks_per_rev, dt);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Differential-drive robot simulation\n";
    std::cout << "  Wheel radius : " << wheel_radius.numerical_value_in(m)  << " m\n";
    std::cout << "  Track width  : " << track_width.numerical_value_in(m)   << " m\n";
    std::cout << "  Cycle time   : " << dt.numerical_value_in(s)            << " s\n\n";

    auto print_pose = [&]() {
        const auto& p = controller.pose();
        std::cout << "  Pose: x=" << p.x.numerical_value_in(m) << " m"
                  << "  y=" << p.y.numerical_value_in(m) << " m"
                  << "  θ=" << p.theta.numerical_value_in(rad) << " rad\n\n";
    };

    // --- Phase 1: drive straight for 2 s ---
    std::cout << "Phase 1: straight forward at 0.5 m/s for 2 s\n";
    for (int i = 0; i < static_cast<int>(2.0 / dt.numerical_value_in(s)); ++i)
        controller.step({0.5 * (m / s), 0.0 * (rad / s)});
    print_pose();

    // --- Phase 2: rotate 90° CCW ---
    std::cout << "Phase 2: rotate 90° CCW at 0.5 rad/s\n";
    const double angle_needed = std::numbers::pi / 2.0;
    const double time_needed  = angle_needed / 0.5;
    const int    steps        = static_cast<int>(time_needed / dt.numerical_value_in(s)) + 1;
    for (int i = 0; i < steps; ++i)
        controller.step({0.0 * (m / s), 0.5 * (rad / s)});
    print_pose();

    // --- Phase 3: drive straight again for 1 s ---
    std::cout << "Phase 3: straight forward at 0.5 m/s for 1 s\n";
    for (int i = 0; i < static_cast<int>(1.0 / dt.numerical_value_in(s)); ++i)
        controller.step({0.5 * (m / s), 0.0 * (rad / s)});
    print_pose();

    std::cout << "Simulation complete.\n";
    return 0;
}
