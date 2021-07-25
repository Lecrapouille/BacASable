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

#ifndef DIMENSIONS_HPP
#  define DIMENSIONS_HPP

#  include <cmath>
#  include <iostream>

// *****************************************************************************
// Graphics
// *****************************************************************************

const float ZOOM = 0.017f;

// *****************************************************************************
// Maths utils
// *****************************************************************************

#  define RAD2DEG(r) ((r) * 57.295779513f)
#  define DEG2RAD(d) ((d) * 0.01745329251994f)

// *****************************************************************************
// https://www.virages.com/Blog/Dimensions-Places-De-Parking
// *****************************************************************************

const float ROAD_WIDTH = 5.0f; // 5 meters

// Places de parking en bataille
const float PARKING_BATAILLE_LENGTH = 5.0f;
const float PARKING_BATAILLE_WIDTH = 2.3f;

// Places de parking en créneau
const float PARKING_CRENEAU_LENGTH = 5.0f;
const float PARKING_CRENEAU_WIDTH = 2.0f;

// Places de parking en épi
const float PARKING_EPI45_LENGTH = 4.8f; // 4.8 m
const float PARKING_EPI45_WIDTH = 2.2f; // 2.2 m
const float PARKING_EPI45_ANGLE = 45.0f; // deg
const float PARKING_EPI60_LENGTH = 5.15f;
const float PARKING_EPI60_WIDTH = 2.25f;
const float PARKING_EPI60_ANGLE = 60.0f; // deg
const float PARKING_EPI75_LENGTH = 5.1f;
const float PARKING_EPI75_WIDTH = 2.25f;
const float PARKING_EPI75_ANGLE = 45.0f; // deg

// ****************************************************************************
// Class holding vehicle dimensions and constants
// ****************************************************************************
class CarDimensions
{
public:

    //--------------------------------------------------------------------------
    //! \brief The turning radius is the radius of the circle created by a
    //! vehicle when it turns with a fixed steering angle. Some formula are given
    //! at http://www.autoturn.ch/giration/standard_r.html
    //! This structure holds these results.
    //--------------------------------------------------------------------------
    struct TurningRadius
    {
        //! \brief turning radius of the virtual wheel located in the middle of
        //! the front axle [meter].
        float R;
        //! \brief turning radius of the internal wheel [meter].
        float Ri;
        //! \brief turning radius of the external wheel [meter].
        float Re;
        //! \brief Largeur balayee [meter]
        float bal;
        //! \brief Surlargeur [meter]
        float sur;
    };

    //--------------------------------------------------------------------------
    //! \brief Define vehicle constants
    //! \param[in] w: car width [meter]
    //! \param[in] l: car length [meter]
    //! \param[in] wb: wheelbase length [meter]
    //! \param[in] bo: back overhang [meter]
    //! \param[in] wr: wheel radius [meter]
    //! \param[in] tc: turning circle [meter]
    //--------------------------------------------------------------------------
    CarDimensions(const char* n,
                  const float w, const float l, const float wb,
                  const float bo, const float wr, const float /*tc*/)
        : name(n), width(w), length(l), wheelbase(wb), back_overhang(bo),
          front_overhang(length - wheelbase - back_overhang), wheel_radius(wr)
    {
        // FIXME
        max_steering_angle = DEG2RAD(30.0f);//28.52f);//asinf(wb / tc);
        //atanf(wb / (tc - w)); //asinf(wb / tc);

        // The more we increase the steering angle, the more turning radii
        // become small. When the steering angle is maximal, the minimum radii
        // will be denoted by (Rmin, Rimin, Remin).
        CarDimensions::TurningRadius res = turningRadius(max_steering_angle);
        Rmin = res.R;
        Rimin = res.Ri;
        Remin = res.Re;

        // Precompute this very used
        Rmintild = Rimin + width / 2.0f;

        // Minimum length of the parallel parking length. See figure 4 "Easy
        // Path Planning and Robust Control for Automatic Parallel Parking" by
        // Sungwoo CHOI, Clément Boussard, Brigitte d’Andréa-Novel.
        //
        // We use the Pythagorean theorem of the triangle CBA (90° on B) where C
        // is the center of the turning circle, B the back-left wheel (internal
        // radius Rimin) and A the front-right wheel (external radius Remin). Since
        // the frame of the car body is placed at the center of the back axle, we
        // have to add the back overhang.
        Lmin = back_overhang + sqrtf(Remin * Remin - Rimin * Rimin);

        std::cout << "=============================" << std::endl;
        std::cout << name << std::endl;
        std::cout << "A: " << RAD2DEG(max_steering_angle) << std::endl;
        std::cout << "Rmin:" << Rmin << std::endl;
        std::cout << "Rimin:" << Rimin << std::endl;
        std::cout << "Remin:" << Remin << std::endl;
        std::cout << "Lmin:" << Lmin << std::endl;
    }

    //--------------------------------------------------------------------------
    //! \brief Compute internal, external turning radii depending on the wheel
    //! angle.
    //! \param[in] wheel_angle [rad]
    //--------------------------------------------------------------------------
    TurningRadius turningRadius(float const wheel_angle)
    {
        const float e = wheelbase;
        const float w = width;
        const float p = front_overhang;

        TurningRadius res;

        // Rayon de braquage [m] (turning radius)
        res.R = e / sinf(wheel_angle);

        // Inner radius [m]
        // Also equals to (e / tanf(wheel_angle) - (w / 2.0f)
        res.Ri = sqrtf(res.R * res.R - e * e) - (w / 2.0f);

        // Outer radius [m]
        res.Re = sqrtf((res.Ri + w) * (res.Ri + w) + (e + p) * (e + p));

        // Largeur balayee [m]
        res.bal = res.Re - res.Ri;

        // surlargeur [m]
        res.sur = res.bal - w;

        return res;
    }

    //! \brief Vehicle name
    std::string name;
    //! \brief Vehicle width [meter]
    float width;
    //! \brief Vehicle length [meter]
    float length;
    //! \brief Empattement [meter]
    float wheelbase;
    //! \brief Porte a faux arriere [meter]
    float back_overhang;
    //! \brief Porte a faux avant [meter]
    float front_overhang;
    //! \brief Rayon roue [meter]
    float wheel_radius;
    //! \brief Epaisseur roue [meter] (only used for the rendering)
    float wheel_width = 0.1f;
    //! \brief Limit of steering angle absolute angle (rad)
    float max_steering_angle;
    //! \brief turning radius of the virtual wheel located in the middle of
    //! the front axle when the steering angle is at maximum.
    float Rmin;
    //! \brief turning radius of the internal wheel when the steering angle is
    //! at maximum.
    float Rimin;
    //! \brief turning radius of the external wheel when the steering angle is
    //! at maximum.
    float Remin;
    //! \brief Precompute Rimin + width / 2 since used many times
    float Rmintild;
    //! \brief Minimal parking length for making a one-trial maneuver parallel
    //! parking ( when the steering angle is at maximum).
    float Lmin;
};

// ****************************************************************************
// Real vehicle dimensions
// ****************************************************************************

// https://www.renault-guyane.fr/cars/TWINGOB07Ph2h/DimensionsEtMotorisations.html
static const CarDimensions TwingoDimensions("Twingo", 1.646f, 3.615f, 2.492f, 0.494f, 0.328f, 10.0f);

// https://www.largus.fr/images/images/ds3-crossback-dimensions-redimensionner.png
static const CarDimensions DS3Dimensions("DS3", 1.79f, 4.118f, 2.558f, 0.7f, 0.328f, 10.4f);

// https://www.vehikit.fr/nissan
static const CarDimensions NV200Dimensions("NV200", 1.219f, 4.321f, 2.725f, 0.840f, 0.241f, 10.6f);

//
static const CarDimensions AudiA6Dimensions("A6", 2.03f, 4.93f, 2.84f, 1.045f, 0.543f, 11.7f);
// 1.898f, 4.938f, 2.905f, 1.113f, 0.543f, 11.7f);

#endif
