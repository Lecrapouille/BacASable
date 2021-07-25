// 2021 Quentin Quadrat lecrapouille@gmail.com
//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <https://unlicense.org>

#include "Car.hpp"
#include "Utils.hpp"

// Note: Car dynamics is not yet made, the current model is dv/dt = a
const bool USE_KINEMATIC = true;

//------------------------------------------------------------------------------
void Car::Kinematic::init(float const x_, float const y_, float const a)
{
    x = x_;
    y = y_;
    heading = a;
}

//------------------------------------------------------------------------------
// http://www.autoturn.ch/giration/standard_r.html
// https://qastack.fr/gamedev/50022/typical-maximum-steering-angle-of-a-real-car
//------------------------------------------------------------------------------
void Car::Kinematic::update(float const dt, Control const& control)
{
    float const wheel_angle = control.outputs.wheel_angle;
    float const body_speed = control.outputs.body_speed;

    heading += dt * body_speed * tanf(wheel_angle) / dim.wheelbase;
    x += dt * body_speed * cosf(heading);
    y += dt * body_speed * sinf(heading);
}

//------------------------------------------------------------------------------
bool Car::Trajectory::computePathPlanning(float const car_x, float const car_y,
                                          float const parking_x, float const parking_y)
{
    // TODO Missing the case the car is not enough far away

    // Initial car position: current position of the car
    Xi = car_x;
    Yi = car_y;

    // Final destination: the parking slot
    Xf = parking_x;
    Yf = parking_y;

    // C1: center of the ending turn (end position of the 2nd turning maneuver)
    Xc1 = Xf + dim.back_overhang;
    Yc1 = Yf + dim.Rmintild;

    // C2: center of the starting turn (begining position of the 1st turning
    // maneuver). Note: the X-coordinate cannot yet be computed.
    Yc2 = Yi - dim.Rmintild;

    // Tengant intersection of C1 and C2.
    Yt = (Yc1 + Yc2) / 2.0f;
    float d = dim.Rmintild * dim.Rmintild - (Yt - Yc1) * (Yt - Yc1);
    if (d < 0.0f)
    {
       std::cerr << "Car is too far away on Y-axis (greater than its turning radius)"
                 << std::endl;
       return false;
    }
    Xt = Xc1 + sqrt(d);

    // Position of the car for starting the 1st turn.
    Xs = 2.0f * Xt - Xc1;
    Ys = Yi;

    // X position of C1.
    Xc2 = Xs;

    // Minimal central angle for making the turn = atanf((Xt - Xc1) / (Yc1 - Yt))
    min_central_angle = atan2f(Xt - Xc1, Yc1 - Yt);

    std::cout << "=============================" << std::endl;
    std::cout << "Initial position: " << Xi << " " << Yi << std::endl;
    std::cout << "Turning at position: " << Xs << " " << Ys << std::endl;
    std::cout << "Center 1st turn: " << Xc2 << " " << Yc2 << std::endl;
    std::cout << "Center 2nd turn: " << Xc1 << " " << Yc1 << std::endl;
    std::cout << "Touching point: " << Xt << " " << Yt << std::endl;
    std::cout << "Angle: " << RAD2DEG(min_central_angle) << std::endl;
    std::cout << "Final position: " << Xf << " " << Yf << std::endl;

    return true;
}

//------------------------------------------------------------------------------
void Car::Trajectory::generateReferenceTrajectory(float const vmax, float const ades)
{
    // Clear internal states
    time = 0.0f;
    m_body_speeds.clear();
    m_body_accelerations.clear();
    m_wheel_angles.clear();

    // Be sure to use basolute values
    float const VMAX = std::abs(vmax); // Max valocity [m/s]
    float const ADES = std::abs(ades); // Desired acceleration [m/s/s]
    assert(vmax > 0.0f);
    assert(ades > 0.0f);

    if (USE_KINEMATIC)
    {
        // Duration to turn front wheels to the maximal angle [s]
        float const DURATION_TO_TURN_WHEELS = 0.0f;

        // Init reference to idle the car.
        add_reference(m_body_speeds, 0.0f, DURATION_TO_TURN_WHEELS);
        add_reference(m_wheel_angles, 0.0f, DURATION_TO_TURN_WHEELS);

        // Init car position -> position when starting the 1st turn
        float t = std::abs(Xi - Xs) / VMAX;
        add_reference(m_body_speeds, -VMAX, t);
        add_reference(m_wheel_angles, 0.0f, t);

        // Pause the car to turn the wheel
        add_reference(m_body_speeds, 0.0f, DURATION_TO_TURN_WHEELS);
        add_reference(m_wheel_angles, -dim.max_steering_angle, DURATION_TO_TURN_WHEELS);

        // 1st turn: start position -> P
        t = arc_length(min_central_angle, dim.Rmintild) / VMAX;
        add_reference(m_body_speeds, -VMAX, t);
        add_reference(m_wheel_angles, -dim.max_steering_angle, t);

        // Pause the car to turn the wheel
        add_reference(m_body_speeds, 0.0f, DURATION_TO_TURN_WHEELS);
        add_reference(m_wheel_angles, dim.max_steering_angle, DURATION_TO_TURN_WHEELS);

        // 2nd turn: P -> final
        add_reference(m_body_speeds, -VMAX, t);
        add_reference(m_wheel_angles, dim.max_steering_angle, t);

        // Pause the car to turn the wheel
        add_reference(m_body_speeds, 0.0f, DURATION_TO_TURN_WHEELS);
        add_reference(m_wheel_angles, 0.0f, DURATION_TO_TURN_WHEELS);

        // Centering in the parking spot
        t = std::abs((PARKING_BATAILLE_LENGTH - dim.length) / 2.0f) / VMAX;
        add_reference(m_body_speeds, VMAX, t);
        add_reference(m_wheel_angles, 0.0f, t);

        // Init reference to idle the car
        add_reference(m_body_speeds, 0.0f, 0.0f);
        add_reference(m_wheel_angles, 0.0f, 0.0f);
    }
    else
    {
        // Kinematics equations
        // http://www.ilectureonline.com/lectures/subject/PHYSICS/1/9/259
        //   acceleration: a, time: t
        //   velocity: v = v0 + a * t
        //   position: x = x0 + v0 * t + 0.5 * a * t * t
        // Initial values:
        //   x0 = 0 since we deal distances.
        //   v0 = 0 since to do its maneuver the car has to halt.
        // Therefore:
        //   v = a t
        //   x = a t^2 / 2
        float const t1 = VMAX / ADES;
        float const d1 = 0.5f * ADES * t1 * t1;

        // Arc length for doing a turn and time needed at constant velocity Vmax for
        // doing the turn.
        float const d2 = arc_length(min_central_angle, dim.Rmintild);
        float const t2 = (d2 - 2.0f * d1) / VMAX;

        // Maneuver Xi => Xs
        float const di = std::abs(Xi - Xs);
        float const ti = (di - 2.0f * d1) / VMAX;

        // Maneuver for centering the car in the spot
        float const d3 = std::abs((PARKING_BATAILLE_LENGTH - dim.length) / 2.0f);
        float const t3 = d3 / VMAX;

        // Duration to turn front wheels to the maximal angle [s]
        float const DURATION_TO_TURN_WHEELS = 1.5f;
        float const AW = dim.max_steering_angle / DURATION_TO_TURN_WHEELS; // [rad/s]

        // Init
        add_reference(m_body_accelerations, 0.0f, 0.0f);
        add_reference(m_wheel_angles, 0.0f, 0.0f);

        // Init car position -> position when starting the 1st turn
        add_reference(m_body_accelerations, (Xi >= Xs ? -ADES : ADES), t1);
        add_reference(m_wheel_angles, 0.0f, t1);

        add_reference(m_body_accelerations, 0.0f, ti);
        add_reference(m_wheel_angles, 0.0f, ti);

        add_reference(m_body_accelerations, (Xi >= Xs ? ADES : -ADES), t1);
        add_reference(m_wheel_angles, 0.0f, t1);

        // Turn wheel
        add_reference(m_body_accelerations, 0.0f, DURATION_TO_TURN_WHEELS);
        add_reference(m_wheel_angles, -AW, DURATION_TO_TURN_WHEELS);

        // Turn 1
        add_reference(m_body_accelerations, -ADES, t1);
        add_reference(m_wheel_angles, 0.0f, t1);
        add_reference(m_body_accelerations, 0.0f, t2);
        add_reference(m_wheel_angles, 0.0f, t2);
        add_reference(m_body_accelerations, ADES, t1);
        add_reference(m_wheel_angles, 0.0f, t1);

        // Turn wheel
        add_reference(m_body_accelerations, 0.0f, DURATION_TO_TURN_WHEELS);
        add_reference(m_body_accelerations, 0.0f, DURATION_TO_TURN_WHEELS);
        add_reference(m_wheel_angles, AW, DURATION_TO_TURN_WHEELS);
        add_reference(m_wheel_angles, AW, DURATION_TO_TURN_WHEELS);

        // Turn 2
        add_reference(m_body_accelerations, -ADES, t1);
        add_reference(m_wheel_angles, 0.0f, t1);
        add_reference(m_body_accelerations, 0.0f, t2);
        add_reference(m_wheel_angles, 0.0f, t2);
        add_reference(m_body_accelerations, ADES, t1);
        add_reference(m_wheel_angles, 0.0f, t1);

        // Turn wheel
        add_reference(m_body_accelerations, 0.0f, DURATION_TO_TURN_WHEELS);
        add_reference(m_wheel_angles, -AW, DURATION_TO_TURN_WHEELS);

        // Centering in the parking spot
        add_reference(m_body_accelerations, ADES, t3);
        add_reference(m_wheel_angles, 0.0f, t3);
        add_reference(m_body_accelerations, -ADES, t3);
        add_reference(m_wheel_angles, 0.0f, t3);

        // Final
        add_reference(m_body_accelerations, 0.0f, DURATION_TO_TURN_WHEELS);
        add_reference(m_wheel_angles, 0.0f, DURATION_TO_TURN_WHEELS);
    }
}

//------------------------------------------------------------------------------
void Car::Trajectory::update(float const dt, Control& control)
{
    time += dt;

    if (USE_KINEMATIC)
    {
        control.ref.body_speed = get_reference(m_body_speeds, time);
        control.ref.wheel_angle = get_reference(m_wheel_angles, time);
    }
    else
    {
        float ab = get_reference(m_body_accelerations, time);
        float aw = get_reference(m_wheel_angles, time);

        std::cout << time << " " << ab << " " << aw << "; " << std::endl;

        control.ref.body_speed += ab * dt;
        control.ref.wheel_angle += aw * dt;
    }
}

//------------------------------------------------------------------------------
void Car::Trajectory::draw(Car const& car, sf::RenderTarget& target)
{
    target.draw(Circle(Xc1, Yc1, dim.Rmintild, sf::Color::Red));
    target.draw(Arrow(Xc1, Yc1, Xc1, Yf, sf::Color::Red));
    target.draw(Circle(Xc2, Yc2, dim.Rmintild, sf::Color::Blue));
    target.draw(Arrow(Xc2, Yc2, Xs, Ys, sf::Color::Blue));
    target.draw(Arrow(Xi, Yi, Xs, Ys, sf::Color::Black));

    target.draw(Circle(Xt, Yt, 2*ZOOM, sf::Color::Yellow));
    target.draw(Circle(Xc1, Yf, 2*ZOOM, sf::Color::Yellow));
    target.draw(Circle(car.x(), car.y(), 2*ZOOM, sf::Color::Black));
}

//------------------------------------------------------------------------------
Car::Car(CarDimensions const& d, float x, float y, float a)
    : dim(d), m_trajectory(d), m_kinematic(d), m_shape(d)
{
    m_kinematic.init(x, y, a);
}

//------------------------------------------------------------------------------
static float estimate_parking_length() // TODO estimate distance between vehicles
{
    ParkingCreneauShape p(0,0);

    return p.length();
}

//------------------------------------------------------------------------------
bool Car::parallelParking(float const parking_x, float const parking_y, float const vmax, float const ades)
{
    // Has the parking spot has enough length to perform a one-trial maneuver parking ?
    if (dim.Lmin >= estimate_parking_length())
    {
        if (!m_trajectory.computePathPlanning(x(), y(), parking_x, parking_y))
            return false;

        m_trajectory.generateReferenceTrajectory(vmax, ades);

        // To Julia/Scilab for plotting
        debug(m_trajectory.accelerations(), "tA", "A");
        debug(m_trajectory.wheel_angles(), "tW", "W");

        return true;
    }
    else
    {
        // Implement "Estimation et contrôle pour le pilotage automatique de
        // véhicule : Stop&Go et parking automatique"
        return false;
    }
}

//------------------------------------------------------------------------------
void Car::update(float const dt)
{
    m_trajectory.update(dt, m_control);
    m_control.update(dt);
    m_kinematic.update(dt, m_control);
    m_shape.update(m_kinematic.x, m_kinematic.y,
                   m_kinematic.heading, m_control.outputs.wheel_angle);
}

//------------------------------------------------------------------------------
void Car::draw(sf::RenderTarget& target)
{
    target.draw(m_shape);
    //drawTurningRadius(target);
    m_trajectory.draw(*this, target);
}

//------------------------------------------------------------------------------
void Car::drawTurningRadius(sf::RenderTarget& target)
{
    float const x = m_kinematic.x;
    float const y = m_kinematic.y;
    float const angle = m_kinematic.heading;

    // Center of the circle
    float L = dim.width / 2 + dim.Rimin;
    float cx = x - L * sinf(angle);
    float cy = y + L * cosf(angle);

    // Internal radius (back-right wheel)
    {
        float K = dim.width / 2 - dim.wheel_width / 2;
        float wx = x - K * sinf(angle);
        float wy = y + K * cosf(angle);
        target.draw(Arrow(cx, cy, wx, wy, sf::Color::Red));
        target.draw(Circle(cx, cy, dim.Rimin, sf::Color::Red));
    }

    // External radius (front-left wheel)
    {
        float K = dim.width / 2 - dim.wheel_width / 2;
        float wx = x + (dim.wheelbase + dim.front_overhang) * cosf(angle) + K * sinf(angle);
        float wy = y + (dim.wheelbase + dim.front_overhang) * sinf(angle) - K * cosf(angle);
        target.draw(Arrow(cx, cy, wx, wy, sf::Color::Blue));
        target.draw(Circle(cx, cy, dim.Remin, sf::Color::Blue));
    }

    // Turning radius of the fake front rear wheel (middle of the axle)
    {
        target.draw(Circle(cx, cy, dim.Rmin, sf::Color::Black));
    }
}
