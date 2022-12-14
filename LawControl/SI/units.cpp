//******************************************************************************
// Test https://github.com/nholthaus/units a compile-time, header-only,
// dimensional analysis and unit conversion library built on C++ with no
// dependencies.
//******************************************************************************

#include "units.h"

using namespace units::literals;
using namespace units::length;
using namespace units::time;
using namespace units::velocity;
using namespace units::angle;

//******************************************************************************
//! \brief Simple car kinematic using the tricycle kinematic equations using
//! https://github.com/nholthaus/units for international system of units library.
//! Formula: http://msl.cs.uiuc.edu/planning/node658.html
//! The position (x, y) of the car is the middle of the rear axle.
//******************************************************************************
class CarUnits
{
public:

    CarUnits(meter_t const wheelbase)
        : m_wheelbase(wheelbase)
    {}

    //! \brief Kinematic step
    //! \param[in] dt: time step
    //! \param[in] angle: wheel angle
    //! \param[in] speed: vehicle speed
    void update(second_t const dt, radian_t const angle, meters_per_second_t const speed)
    {
        auto u = 1.0_rad;

        m_speed = speed;
        m_heading += (dt * (speed / m_wheelbase) * u * units::math::tan(angle));
        m_x += dt * speed * units::math::cos(m_heading);
        m_y += dt * speed * units::math::sin(m_heading);
    }

    inline meter_t x() const { return m_x; }
    inline meter_t y() const { return m_y; }
    inline meters_per_second_t speed() const { return m_speed; }
    inline radian_t heading() const { return m_heading; }

private:

    //! \brief longitudinal wheel to wheel distance
    const meter_t m_wheelbase;
    //! \brief X position of the middle of the rear axle.
    meter_t m_x = 0.0_m;
    //! \brief X position of the middle of the rear axle.
    meter_t m_y = 0.0_m;
    //! \brief Heading of the vehicle.
    radian_t m_heading = 0.0_rad;
    //! \brief Longitudinal speed of the vehicle.
    meters_per_second_t m_speed;
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
// g++ --std=c++17 -Wall -Wextra -Iunits/include class.cpp -o prog
//******************************************************************************
int main()
{
    const meter_t wheelbase = 1.0_m;
    CarUnits car1(wheelbase);
    CarNoUnits car2(wheelbase.value());

    radian_t angle = 10.0_deg;
    meters_per_second_t speed = 10.0_km / 1.0_hr;
    second_t dt = 0.1_s;

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
