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

#ifndef MAIN_HPP
#  define MAIN_HPP

#  include "GUI.hpp"
#  include "Car.hpp"

// *****************************************************************************
//! \brief
// *****************************************************************************
class AutoParkingGUI: public GUIStates
{
public:

    AutoParkingGUI(Application& application);

    ~AutoParkingGUI()
    {
        renderer().close();
    }

private: // Derived from GUI

    //--------------------------------------------------------------------------
    //! \brief Inherit from GUI class. Draw the chessboard and pieces.
    //--------------------------------------------------------------------------
    virtual void draw(const float /*dt*/) override;

    //--------------------------------------------------------------------------
    //! \brief Inherit from GUI class. Update GUI.
    //----------------------------------/---------------------------------------
    virtual void update(const float dt) override;

    //--------------------------------------------------------------------------
    //! \brief Inherit from GUI class. Manage mouse and keyboard events.
    //--------------------------------------------------------------------------
    virtual void handleInput() override;

    //--------------------------------------------------------------------------
    //! \brief Inherit from GUI class. Return true if GUI is alive.
    //--------------------------------------------------------------------------
    virtual bool isRunning() override
    {
        return m_running;
    }

    //--------------------------------------------------------------------------
    //! \brief Called when the GUI has been enabled.
    //--------------------------------------------------------------------------
    virtual void activate() override
    {
        // Do nothing
    }

    //--------------------------------------------------------------------------
    //! \brief Called when the GUI has been disabled.
    //--------------------------------------------------------------------------
    virtual void deactivate() override
    {
        // Do nothing
    }

private:

    sf::View m_view;
    WorldShape m_world;
    Car m_car;
    std::atomic<bool> m_running{true};
    sf::Vector2f m_mouse;
};

#endif
