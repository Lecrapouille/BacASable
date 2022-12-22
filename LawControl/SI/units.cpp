//******************************************************************************
// Test https://github.com/nholthaus/units a compile-time, header-only,
// dimensional analysis and unit conversion library built on C++ with no
// dependencies.
//******************************************************************************

#include "units.h"

using namespace units::literals;
using Meter = units::length::meter_t;
using Radian = units::angle::radian_t;
using MeterPerSecond = units::velocity::meters_per_second_t;
using Second = units::time::second_t;

//******************************************************************************
//! \brief Simple car kinematic using the tricycle kinematic equations using
//! https://github.com/nholthaus/units for international system of units library.
//! Formula: http://msl.cs.uiuc.edu/planning/node658.html
//! The position (x, y) of the car is the middle of the rear axle.
//******************************************************************************
class CarUnits
{
public:

    CarUnits(Meter const wheelbase)
        : m_wheelbase(wheelbase)
    {}

    //! \brief Kinematic step
    //! \param[in] dt: time step
    //! \param[in] angle: wheel angle
    //! \param[in] speed: vehicle speed
    void update(Second const dt, Radian const angle, MeterPerSecond const speed)
    {
        auto u = 1.0_rad;

        m_speed = speed;
        m_heading += (dt * (speed / m_wheelbase) * u * units::math::tan(angle));
        m_x += dt * speed * units::math::cos(m_heading);
        m_y += dt * speed * units::math::sin(m_heading);
    }

    inline Meter x() const { return m_x; }
    inline Meter y() const { return m_y; }
    inline MeterPerSecond speed() const { return m_speed; }
    inline Radian heading() const { return m_heading; }

private:

    //! \brief longitudinal wheel to wheel distance
    const Meter m_wheelbase;
    //! \brief X position of the middle of the rear axle.
    Meter m_x = 0.0_m;
    //! \brief X position of the middle of the rear axle.
    Meter m_y = 0.0_m;
    //! \brief Heading of the vehicle.
    Radian m_heading = 0.0_rad;
    //! \brief Longitudinal speed of the vehicle.
    MeterPerSecond m_speed;
};

#include <cmath>

//******************************************************************************
//! \brief Simple car kinematic using the tricycle kinematic equations not using
//! SI library.
//******************************************************************************
class CarNoUnits
{
public:

    CarNoUnits(float const w)
        : m_wheelbase(w)
    {}

    void update(float const dt, float const angle, float const speed)
    {
        m_speed = speed;
        m_heading += dt * (speed / m_wheelbase) * std::tan(angle);
        m_x += dt * speed * std::cos(m_heading);
        m_y += dt * speed * std::sin(m_heading);
    }

    inline float x() const { return m_x; }
    inline float y() const { return m_y; }
    inline float speed() const { return m_speed; }
    inline float heading() const { return m_heading; }

private:

    const float m_wheelbase;
    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_heading = 0.0f;
    float m_speed;
};

//******************************************************************************
// g++ --std=c++17 -Wall -Wextra -Iunits/include units.cpp -o prog-units
//******************************************************************************
int main()
{
    const Meter wheelbase = 1.0_m;
    CarUnits car1(wheelbase);
    CarNoUnits car2(wheelbase.value());

    Radian angle = 10.0_deg;
    MeterPerSecond speed = 10.0_km / 1.0_hr;
    Second dt = 0.1_s;

    std::cout << "Step;Time;Car1 Position x;Car1 Position Y;Car1 Speed;Car1 Heading;"
              << "Car2 Position x;Car2 Position Y;Car2 Speed;Car2 Heading"
              << std::endl;
    for (int i = 0; i < 10; ++i)
    {
        car1.update(dt, angle, speed);
        car2.update(dt.value(), angle.value(), speed.value());
        std::cout << i << ";"
                  << (i * dt) << ";"

                  << car1.x() << ";"
                  << car1.y() << ";"
                  << car1.speed() << ";"
                  << car1.heading() << ";"

                  << car2.x() << ";"
                  << car2.y() << ";"
                  << car2.speed() << ";"
                  << car2.heading()

                  << std::endl;
    }

    return 0;
}
