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

#ifndef CAR_HPP
#  define CAR_HPP

#  include "GUI.hpp"
#  include "Dimensions.hpp"
#  include "Shapes.hpp"
#  include <memory>
#  include <atomic>

// *****************************************************************************
//! \brief Helper struct to hold a temporal value used for defining a trajectory.
// *****************************************************************************
struct Pair
{
    Pair(float d, float t)
      : data(d), time(t)
    {}

    float data;
    float time;
};

// *****************************************************************************
//! \brief Model of a car (kinematic and a prototype of auto-parking)
// *****************************************************************************
class Car
{
public:

    // *************************************************************************
    //! \brief Simulate the ECU doing the speed regulation of the car.
    // *************************************************************************
    class Control
    {
    public:

        void init()
        {}

        //----------------------------------------------------------------------
        //! \brief The desired
        //----------------------------------------------------------------------
        struct References
        {
            float body_speed = 0.0f;
            float wheel_angle = 0.0f;
        };

        //----------------------------------------------------------------------
        //! \brief
        //----------------------------------------------------------------------
        struct Inputs
        {
        };

        //----------------------------------------------------------------------
        //! \brief
        //----------------------------------------------------------------------
        struct Outputs
        {
            float body_speed = 0.0f;
            float wheel_angle = 0.0f;
        };

        //----------------------------------------------------------------------
        //! \brief
        //----------------------------------------------------------------------
        void update(float const /*dt*/ /*, sensors*/)
        {
            outputs.body_speed = ref.body_speed;
            outputs.wheel_angle = ref.wheel_angle;
        }

        References ref;
        Outputs outputs;
    };

    // *************************************************************************
    //! \brief Car kinematic (no acceleration, no forces, no mass) since for the
    //! autoparking the car will drive at low speed (therefore no slide effects)
    //! and trajectory does not depend on the speed.
    // *************************************************************************
    class Kinematic
    {
    public:

        Kinematic(CarDimensions const& d)
            : dim(d)
        {}

        // ---------------------------------------------------------------------
        //! \brief Set the initial values of kinematic states: position (XY
        //! coordinates) and orientation of the car (world references).
        //! \param[in] x the X world coordinate of the middle of the back wheel
        //! axle of the car.
        //! \param[in] y the Y world coordinate of the middle of the back wheel
        //! axle of the car.
        //! \param[in] a the heading orientation [rad].
        // ---------------------------------------------------------------------
        void init(float const x, float const y, float const a);

        // ---------------------------------------------------------------------
        //! \brief Refresh the kinematic states depending on the car velocity
        //! and wheel angle.
        //! \param[in] dt: time step from previous call.
        //! \param[in] control: car states.
        // ---------------------------------------------------------------------
        void update(float const dt, Control const& control);

        //! \brief Hold the dimension of the car.
        CarDimensions const& dim;
        //! \brief the X world coordinate of the middle of the back wheel
        //! axle of the car.
        float x = 0.0f;
        //! \brief the Y world coordinate of the middle of the back wheel
        //! axle of the car.
        float y = 0.0f;
        //! \brief Heading of the car [radian].
        float heading = 0.0f;
    };

    // *************************************************************************
    //! \brief Compute the trajectory allowing the car to park. For parallel
    //! parking only!
    // *************************************************************************
    class Trajectory
    {
    public:

        Trajectory(CarDimensions const& d)
            : dim(d)
        {}

        // ---------------------------------------------------------------------
        //! \brief Compute trajectory waypoints.
        //! \param[in] Xi: the X world coordinate of the initial position of the
        //! middle of the back wheel axle of the car at the begining of the
        //! maneuver.
        //! \param[in] Yi: the Y world coordinate of the initial position of the
        //! middle of the back wheel axle of the car at the begining of the
        //! maneuver.
        //! \param[in] Xf: the desired X world coordinate of the final
        //! destination in where the middle of the back wheel axle of the car
        //! has to be at the end of the maneuver.
        //! \param[in] Yf: the desired Y world coordinate of the final
        //! destination in where the middle of the back wheel axle of the car
        //! has to be at the end of the maneuver.
        // ---------------------------------------------------------------------
        bool computePathPlanning(float const Xi, float const Yi,
                                 float const Xf, float const Yf);

        // ---------------------------------------------------------------------
        //! \brief Generated tables of timed references for the auto-pilot to
        //! follow the desired trajectory computed by computePathPlanning().
        //! \param[in] vmax: maximal velocity [m/s] for the car.
        //! \param[in] ades: desired acceleration [m/s^2] for the car.
        // ---------------------------------------------------------------------
        void generateReferenceTrajectory(float const vmax, float const ades);

        // ---------------------------------------------------------------------
        //! \brief Return Timed references of the car accelerations.
        // ---------------------------------------------------------------------
        inline std::vector<Pair> const& accelerations() const
        {
            return m_body_accelerations;
        }

        // ---------------------------------------------------------------------
        //! \brief Return Timed references of the car speeds.
        // ---------------------------------------------------------------------
        inline std::vector<Pair> const& speeds() const
        {
            return m_body_speeds;
        }

        // ---------------------------------------------------------------------
        //! \brief Return Timed references of the front wheel angles.
        // ---------------------------------------------------------------------
        inline std::vector<Pair> const& wheel_angles() const
        {
            return m_wheel_angles;
        }

        // ---------------------------------------------------------------------
        //! \brief Read the timed references need by the car auto-pilot.
        //! \param[in] dt: time step from previous call.
        //! \param[out] control: the auto-pilot.
        // ---------------------------------------------------------------------
        void update(float const dt, Control& control);

        // ---------------------------------------------------------------------
        //! \brief Debug. Draw the trajectory.
        // ---------------------------------------------------------------------
        void draw(Car const& car, sf::RenderTarget& target);

        // ---------------------------------------------------------------------
        //! \brief Debug. Data to be plotted in Julia
        // ---------------------------------------------------------------------
        void debug();

    private:

        //! \brief Hold the dimension of the car.
        CarDimensions const& dim;
        //! \brief Integration time
        float time = 0.0f;
        //! \brief X-Y world coordinates:
        // s: initial car position
        // f: final car position
        // s: starting position for turning
        // c1: Center of the circle 1
        // c2: Center of the circle 2
        // t: tangential intersection point between C1 and C2
        float Xc1, Yc1, Xc2, Yc2, Xt, Yt, Xs, Ys, Xi, Yi, Xf, Yf;
        //! \brief Minimal central angle for making the turn.
        float min_central_angle;
        //! \brief Timed reference for the car accelerations.
        std::vector<Pair> m_body_accelerations;
        //! \brief Timed reference for the car speeds.
        std::vector<Pair> m_body_speeds;
        //! \brief Timed reference for front wheel angles.
        std::vector<Pair> m_wheel_angles;
    };

    // ---------------------------------------------------------------------
    //! \brief Set the initial values of kinematic states: position (XY
    //! coordinates) and orientation of the car (world references).
    //! \param[in] x the X world coordinate of the middle of the back wheel
    //! axle of the car.
    //! \param[in] y the Y world coordinate of the middle of the back wheel
    //! axle of the car.
    //! \param[in] a the heading orientation [rad].
    // ---------------------------------------------------------------------
    Car(CarDimensions const& dim, float x, float y, float a);

    // ---------------------------------------------------------------------
    //! \brief Compute trajectory waypoints and references.
    //! \param[in] car: need some information from the car (position,
    //! dimension).
    //! \param[in] parking_x: the desired X world coordinate of the final
    //! destination in where the middle of the back wheel axle of the car
    //! has to be at the end of the maneuver.
    //! \param[in] parking_y: the desired Y world coordinate of the final
    //! destination in where the middle of the back wheel axle of the car
    //! has to be at the end of the maneuver.
    //! \param[in] vmax: maximal velocity [m/s] for the car.
    //! \param[in] ades: desired acceleration [m/s^2] for the car.
    // ---------------------------------------------------------------------
    bool parallelParking(float const parking_x, float const parking_y,
                         float const vmax, float const ades);

    // ---------------------------------------------------------------------
    //! \brief
    // ---------------------------------------------------------------------
    void update(float const dt);

    // ---------------------------------------------------------------------
    //! \brief Draw the car shape and debug trajectory ...
    // ---------------------------------------------------------------------
    void draw(sf::RenderTarget& target);

    // ---------------------------------------------------------------------
    //! \brief Get the X world coordinate of the middle of the back wheel
    //! axle of the car.
    // ---------------------------------------------------------------------
    float x() const { return m_kinematic.x; }

    // ---------------------------------------------------------------------
    //! \brief Get the Y world coordinate of the middle of the back wheel
    //! axle of the car.
    // ---------------------------------------------------------------------
    float y() const { return m_kinematic.y; }

    // ---------------------------------------------------------------------
    //! \brief Get the heading of the car [radian].
    // ---------------------------------------------------------------------
    float heading() const { return m_kinematic.heading; }

    // ---------------------------------------------------------------------
    //! \brief Get the velocity of the car [m/s]
    // ---------------------------------------------------------------------
    float speed() const { return m_control.ref.body_speed; }

    // ---------------------------------------------------------------------
    //! \brief Get the wheel angle [radian].
    // ---------------------------------------------------------------------
    float wheel_angle() const { return m_control.ref.wheel_angle; }

    // ---------------------------------------------------------------------
    //! \brief Get the SFML shape for the rendering
    // ---------------------------------------------------------------------
    CarShape const& shape() const { return m_shape; }

protected:

    // ---------------------------------------------------------------------
    //! \brief Draw the trajectory
    // ---------------------------------------------------------------------
    void drawTurningRadius(sf::RenderTarget& target);

public:

    //! \brief Hold the dimension of the car.
    CarDimensions const& dim;

protected:

    //! \brief Computer the car trajectory for parking
    Trajectory m_trajectory;
    //! \brief For the simulation only: Kinematic of the car for the rendering.
    Kinematic m_kinematic;
    //! \brief For the simulation only: control the car.
    Control m_control;
    //! \brief For the simulation only: Shape of the car for the rendereing with SFML
    CarShape m_shape;
};

#endif
