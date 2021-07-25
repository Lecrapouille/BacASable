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

#ifndef SHAPES_HPP
#  define SHAPES_HPP

// *****************************************************************************
//! \file Just draw figures with the the SFML graphic library: car, parking,
//! world.
// *****************************************************************************

#  include <SFML/Graphics.hpp>
#  include <iostream>
#  include "Dimensions.hpp"

    static inline float norm(const float xa, const float ya, const float xb, const float yb)
    {
        return sqrtf((xb - xa) * (xb - xa) + (yb - ya) * (yb - ya));
    }

// *****************************************************************************
//! \brief Circle used to draw turning radii
// *****************************************************************************
class Circle : public sf::Drawable
{
public:

    Circle(float x, float y, float r, sf::Color color = sf::Color::Red)
    {
        m_shape.setRadius(r);
        m_shape.setOrigin(m_shape.getRadius(), m_shape.getRadius());
        m_shape.setPosition(x, y);
        m_shape.setFillColor(sf::Color(255, 255, 255, 0));
        m_shape.setOutlineThickness(ZOOM * 2.0f);
        m_shape.setOutlineColor(color);
        m_shape.setPointCount(100);
    }

private:

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override final
    {
        target.draw(m_shape, states);
    }

private:

    sf::CircleShape m_shape;
};

// *****************************************************************************
//! \brief Draw an arrow
// *****************************************************************************
class Arrow : public sf::Drawable
{
public:

    Arrow(const float xa, const float ya, const float xb, const float yb,
          sf::Color color)
    {
        // Arc magnitude
        const float arrowLength = norm(xa, ya, xb, yb);

        // Orientation
        const float teta = (yb - ya) / (xb - xa);
        float arrowAngle = std::atan(teta) * 180.0f / 3.1415f; // rad -> deg
        if (xb < xa)
            arrowAngle += 180.f;
        else if (yb < ya)
            arrowAngle += 360.f;

        // Head of the arrow
        const sf::Vector2f arrowHeadSize{ ZOOM * 9.0f, ZOOM * 9.0f };
        m_head = sf::ConvexShape{ 3 };
        m_head.setPoint(0, { 0.f, 0.f });
        m_head.setPoint(1, { arrowHeadSize.x, arrowHeadSize.y / 2.f });
        m_head.setPoint(2, { 0.f, arrowHeadSize.y });
        m_head.setOrigin(arrowHeadSize.x, arrowHeadSize.y / 2.f);
        m_head.setPosition(sf::Vector2f(xb, yb));
        m_head.setRotation(arrowAngle);
        m_head.setOutlineThickness(ZOOM);
        m_head.setOutlineColor(color);
        m_head.setFillColor(color);

        // Tail of the arrow.
        const sf::Vector2f tailSize{ arrowLength - arrowHeadSize.x, ZOOM * 1.0f };
        m_tail = sf::RectangleShape{ tailSize };
        m_tail.setOrigin(0.f, tailSize.y / 2.f);
        m_tail.setPosition(sf::Vector2f(xa, ya));
        m_tail.setRotation(arrowAngle);
        m_tail.setOutlineThickness(ZOOM);
        m_tail.setOutlineColor(color);
        m_tail.setFillColor(color);
    }

private:

    void draw(sf::RenderTarget& target, sf::RenderStates /*states*/) const override final
    {
        target.draw(m_tail);
        target.draw(m_head);
    }

private:

    sf::RectangleShape m_tail;
    sf::ConvexShape m_head;
};

// *****************************************************************************
//! \brief Draw a car shape using the SFML graphics library. The car is rendered
//! as a rectangle and only the front wheel is drawn. The origin of the body is
//! the middle of the rear wheels.
// *****************************************************************************
class CarShape : public sf::Drawable
{
    friend class WorldShape;

public:

    // -------------------------------------------------------------------------
    //! \brief Constructor with no position and no orientation of the shape
    //! are given.
    // -------------------------------------------------------------------------
    CarShape(CarDimensions const& dim);

    // -------------------------------------------------------------------------
    //! \brief Constructor with position and orientation of the shape.
    //! \param[in] bx, by : rear wheel poqition.
    //! \param[in] ba: body orientation (in radian)
    //! \param[in] wa: front wheel orientation (in radian)
    // -------------------------------------------------------------------------
    CarShape(CarDimensions const& dim, float bx, float by, float ba, float wa);

    // -------------------------------------------------------------------------
    //! \brief Reset position and orientation of the shape.
    //! \param[in] bx, by : rear wheel position.
    //! \param[in] ba: body angle (in radian)
    //! \param[in] wa: front wheel angle (in radian)
    // -------------------------------------------------------------------------
    void update(float bx, float by, float ba, float wa);

protected:

    // -------------------------------------------------------------------------
    //! \brief Draw the car: rectangle for the body and rectangle for the front
    //! wheel.
    // -------------------------------------------------------------------------
    void draw(sf::RenderTarget& target, sf::RenderStates /*states*/) const override final;

public:

    //! \brief
    CarDimensions const& dim;

private:

    // FIXME temporary
    sf::CircleShape debug;
    sf::RectangleShape hline;
    sf::RectangleShape vline;

    //! \brief The body drawn as a rectangle.
    sf::RectangleShape body;
    //! \brief The front wheel drawn as a rectangle.
    sf::RectangleShape wheels[4];
};

// *****************************************************************************
//! \brief Draw a parking slot using the SFML graphics library. The origin of
//! the body is the center of the rectangle.
// *****************************************************************************
class ParkingShape : public sf::Drawable
{
    friend class WorldShape;

public:

    // -------------------------------------------------------------------------
    //! \brief Return the dimension of the car body
    // -------------------------------------------------------------------------
    enum Type { Bataille, Creneau, Epi45, Epi60, Epi75 };

    // -------------------------------------------------------------------------
    //! \brief Return the dimension of the car body
    // -------------------------------------------------------------------------
    float length() const { return m_length; }
    float width() const { return m_width; }
    float angle() const { return m_angle; }
    float x() const { return shape.getPosition().x; }
    float y() const { return shape.getPosition().y; }

protected:

    // -------------------------------------------------------------------------
    //! \brief Constructor with position and orientation of the shape.
    //! \param[in] x, y : position of the center of the rectangle.
    //! \param[in] angle: the rotation of the parking slot.
    // -------------------------------------------------------------------------
    ParkingShape(float length, float width, float x, float y, float angle);

    // -------------------------------------------------------------------------
    //! \brief Draw the parking slot: simple oriented rectangle.
    // -------------------------------------------------------------------------
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override final;

private:

    sf::RectangleShape shape;
    float m_length;
    float m_width;
    float m_angle;
};

// *****************************************************************************
//! \brief Parking slot in bataille
// *****************************************************************************
class ParkingBatailleShape : public ParkingShape
{
public:

    ParkingBatailleShape(float x, float y)
        : ParkingShape(PARKING_BATAILLE_LENGTH, PARKING_BATAILLE_WIDTH, x, y, 90.0f)
    {}
};

// *****************************************************************************
//! \brief Parking slot in creneau
// *****************************************************************************
class ParkingCreneauShape : public ParkingShape
{
public:

    ParkingCreneauShape(float x, float y)
        : ParkingShape(PARKING_CRENEAU_LENGTH, PARKING_CRENEAU_WIDTH,
                       x, y, 0.0f)
    {}
};

// *****************************************************************************
//! \brief Parking slot in épi at 45 degree
// *****************************************************************************
class ParkingEpi45Shape : public ParkingShape
{
public:

    ParkingEpi45Shape(float x, float y)
        : ParkingShape(PARKING_EPI45_LENGTH, PARKING_EPI45_WIDTH,
                       x, y, PARKING_EPI45_ANGLE)
    {}
};

// *****************************************************************************
//! \brief Parking slot in épi at 45 degree
// *****************************************************************************
class ParkingEpi60Shape : public ParkingShape
{
public:

    ParkingEpi60Shape(float x, float y)
        : ParkingShape(PARKING_EPI60_LENGTH, PARKING_EPI60_WIDTH,
                        x, y, PARKING_EPI60_ANGLE)
    {}
};

// *****************************************************************************
//! \brief Parking slot in épi at 45 degree
// *****************************************************************************
class ParkingEpi75Shape : public ParkingShape
{
public:

    ParkingEpi75Shape(float x, float y)
        : ParkingShape(PARKING_EPI75_LENGTH, PARKING_EPI75_WIDTH,
                        x, y, PARKING_EPI75_ANGLE)
    {}
};

// *****************************************************************************
//! \brief Draw a world made of parking slots and static cars.
// *****************************************************************************
class WorldShape : public sf::Drawable
{
public:

    void clear()
    {
        parkings.clear();
        parked.clear();
    }

    ParkingShape& addParking(ParkingShape::Type type, float x, float y, bool occupied)
    {
        switch(type)
        {
        case ParkingShape::Type::Bataille:
            parkings.push_back(std::make_unique<ParkingBatailleShape>(x, y));
            break;
        case ParkingShape::Type::Creneau:
            parkings.push_back(std::make_unique<ParkingCreneauShape>(x, y));
            break;
        case ParkingShape::Type::Epi45:
            parkings.push_back(std::make_unique<ParkingEpi45Shape>(x, y));
            break;
        case ParkingShape::Type::Epi60:
            parkings.push_back(std::make_unique<ParkingEpi60Shape>(x, y));
            break;
        case ParkingShape::Type::Epi75:
            parkings.push_back(std::make_unique<ParkingEpi75Shape>(x, y));
            break;
        default:
            std::cerr << "Bad parking enum" << std::endl;
            exit(1);
            break;
        }

        if (occupied)
        {
            float a = parkings.back()->angle();
            parked.push_back(std::make_unique<CarShape>(TwingoDimensions, x, y, a, 0));
        }

        return *parkings.back();
    }

    CarShape& addParkedCar(float bx, float by, float ba)
    {
        parked.push_back(std::make_unique<CarShape>(TwingoDimensions, bx, by, ba, 0));
        return *parked.back();
    }

protected:

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override final;

public:

    std::vector<std::unique_ptr<ParkingShape>> parkings;
    std::vector<std::unique_ptr<CarShape>> parked;
};

#endif
