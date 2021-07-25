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

#include "Shapes.hpp"
#include <cmath>

//------------------------------------------------------------------------------
CarShape::CarShape(CarDimensions const& d)
    : dim(d)
{
    body.setSize(sf::Vector2f(dim.length, dim.width));
    body.setOrigin(dim.back_overhang, body.getSize().y / 2); // Origin on the middle of the rear wheels
    body.setFillColor(sf::Color(165, 42, 42));
    body.setOutlineThickness(ZOOM);
    body.setOutlineColor(sf::Color::Blue);

    for (int i = 0; i < 4; ++i)
    {
        wheels[i].setSize(sf::Vector2f(dim.wheel_radius * 2, dim.wheel_width));
        wheels[i].setOrigin(wheels[i].getSize().x / 2, wheels[i].getSize().y / 2);
        wheels[i].setFillColor(sf::Color::Black);
        wheels[i].setOutlineThickness(ZOOM);
        wheels[i].setOutlineColor(sf::Color::Yellow);
    }

    debug.setRadius(5);
    debug.setOrigin(sf::Vector2f(debug.getRadius(), debug.getRadius()));
    debug.setFillColor(sf::Color::Red);


    hline.setSize(sf::Vector2f(1024, ZOOM));
    hline.setOrigin(hline.getSize().x / 2, hline.getSize().y / 2);
    hline.setFillColor(sf::Color::Black);
    hline.setOutlineThickness(ZOOM);
    hline.setOutlineColor(sf::Color::Black);

    vline.setSize(sf::Vector2f(ZOOM, 600));
    vline.setOrigin(vline.getSize().x / 2, vline.getSize().y / 2);
    vline.setFillColor(sf::Color::Black);
    vline.setOutlineThickness(ZOOM);
    vline.setOutlineColor(sf::Color::Black);
}

//------------------------------------------------------------------------------
CarShape::CarShape(CarDimensions const& dim, float bx, float by, float ba, float wa)
    : CarShape(dim)
{
    update(bx, by, ba, wa);
}

//------------------------------------------------------------------------------
void CarShape::update(float bx, float by, float ba, float wa)
{
    float wx, wy;

    // Wheel offset along the Y-axis
    const float K = dim.width / 2 - dim.wheel_width / 2;

    body.setRotation(RAD2DEG(ba));
    body.setPosition(bx, by);

    //debug
    hline.setPosition(bx, by);
    vline.setPosition(bx, by);
    debug.setPosition(27.7*2, by);

    // front-right wheel
    wx = bx + dim.wheelbase * cosf(ba) - K * sinf(ba);
    wy = by + dim.wheelbase * sinf(ba) + K * cosf(ba);
    wheels[0].setPosition(wx, wy);
    wheels[0].setRotation(RAD2DEG(ba + wa));

    // front-left wheel
    wx = bx + dim.wheelbase * cosf(ba) + K * sinf(ba);
    wy = by + dim.wheelbase * sinf(ba) - K * cosf(ba);
    wheels[1].setPosition(wx, wy);
    wheels[1].setRotation(RAD2DEG(ba + wa));

    // back-right wheel
    wx = bx - K * sinf(ba);
    wy = by + K * cosf(ba);
    wheels[2].setPosition(wx, wy);
    wheels[2].setRotation(RAD2DEG(ba));

    // back-left wheel
    wx = bx + K * sinf(ba);
    wy = by - K * cosf(ba);
    wheels[3].setPosition(wx, wy);
    wheels[3].setRotation(RAD2DEG(ba));
}

//------------------------------------------------------------------------------
void CarShape::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(body, states);
    target.draw(wheels[0], states);
    target.draw(wheels[1], states);
    target.draw(wheels[2], states);
    target.draw(wheels[3], states);

    //target.draw(debug, states);

    //target.draw(vline, states);
    //target.draw(hline, states);
}

//------------------------------------------------------------------------------
ParkingShape::ParkingShape(float length, float width, float x, float y, float angle)
    : m_length(length), m_width(width), m_angle(angle)
{
    shape.setSize(sf::Vector2f(length, width));
    shape.setOrigin(shape.getSize().x / 2, shape.getSize().y / 2);
    shape.setPosition(x,y);
    shape.setRotation(angle);
    shape.setFillColor(sf::Color::White);
    shape.setOutlineThickness(ZOOM);
    shape.setOutlineColor(sf::Color::Black);
}

//------------------------------------------------------------------------------
void ParkingShape::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(shape, states);
}

//------------------------------------------------------------------------------
void WorldShape::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    for (auto const& it: parkings)
    {
        it->draw(target, states);
    }

    for (auto const& it: parked)
    {
        it->draw(target, states);
    }
}
