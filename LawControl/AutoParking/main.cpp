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

#include "main.hpp"
#include <iostream>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

//------------------------------------------------------------------------------
AutoParkingGUI::AutoParkingGUI(Application& application)
    : GUIStates("Auto Parking", application.renderer()),
      m_car(TwingoDimensions, 105.0f, 102.0f, DEG2RAD(0))
{
    m_view = renderer().getDefaultView();
    m_view.setSize(WINDOW_WIDTH, -WINDOW_HEIGHT);
    m_view.setCenter(m_car.x(), m_car.y());
    m_view.zoom(ZOOM);
    renderer().setView(m_view);

    int x = 100.0f;
    int y = 100.0f;
    for (int i = 0; i < 10; ++i)
    {
        ParkingShape& parking = m_world.addParking(ParkingShape::Type::Creneau, x, y, false);
        x += parking.length();
    }

    // Final destination
   static float const parking_x = 97.5f;
   static float const parking_y = 100.0f;
   static float const max_speed = 1.0f;//* 0.277778f ; // [m/s]
   static float const acceleration = 1.0f; // [m/s/s]
   if (!m_car.parallelParking(parking_x, parking_y, max_speed, acceleration))
   {
       std::cerr << "The car cannot park" << std::endl;
   }
}

//------------------------------------------------------------------------------
void AutoParkingGUI::handleInput()
{
    sf::Event event;
    m_mouse = sf::Vector2f(sf::Mouse::getPosition(renderer()));

    while (m_running && renderer().pollEvent(event))
    {
        switch (event.type)
        {
        case sf::Event::Closed:
            m_running = false;
            break;
        case sf::Event::KeyPressed:
            if (event.key.code == sf::Keyboard::Escape)
            {
                m_running = false;
            }
            break;
        default:
            break;
        }
    }
}

//------------------------------------------------------------------------------
void AutoParkingGUI::update(const float dt)
{
    m_view.setCenter(m_car.x(), m_car.y()); renderer().setView(m_view);
    m_car.update(dt);
}

//------------------------------------------------------------------------------
void AutoParkingGUI::draw(const float /*dt*/)
{
    renderer().draw(m_world);
    m_car.draw(renderer());
}

// -----------------------------------------------------------------------------
int main()
{
    Application app(WINDOW_WIDTH, WINDOW_HEIGHT, "Auto Parking");
    AutoParkingGUI gui(app);
    gui.bgColor = sf::Color(255,255,255,255);

    try
    {
        app.push(gui);
        app.loop();
    }
    catch (std::string const& msg)
    {
        std::cerr << "Fatal: " << msg << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
