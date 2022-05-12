// This code is a modification of the original project that can be found at
// https://github.com/Lecrapouille/OpenGLCppWrapper

#ifndef GLWINDOW_HPP
#  define GLWINDOW_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

// *****************************************************************************
//! \brief Base class for creating OpenGL window. This class create the OpenGL
//! context, create the windows and call private virtual methods setup() and
//! update(). This must be derived to implement setup() and update().
// *****************************************************************************
class GLWindow
{
public:

    GLWindow(uint32_t const width, uint32_t const height, const char *title);
    virtual ~GLWindow();

    //! \brief call setup() once and if succeeded call update() within a runtime
    //! loop. Return false in case of failure.
    bool start();

    inline float dt() const { return m_deltaTime; }

    void SetFBSize(uint32_t width, uint32_t height) {
        m_fb_width = width; m_fb_height = height;
    }

    void SetMousePos(uint32_t x, uint32_t y) {
        m_mouse_x = x*m_fb_width/m_width;
        m_mouse_y = y*m_fb_height/m_height;
        std::cout << "mouse: x=" << m_mouse_x << ", y=" << m_mouse_y << std::endl;
    }

    void SetMouseScroll(double xoffset, double yoffset) {
        m_scroll_x = float(xoffset);
        m_scroll_y = float(yoffset);
        std::cout << "scroll: dx=" << m_scroll_x << ", dy=" << m_scroll_y << std::endl;
    }

private:

    void init();

    //! \brief Implement the init for your application. Return false in case of failure.
    virtual bool setup() = 0;

    //! \brief Implement the update for your application. Return false in case of failure.
    virtual bool update() = 0;

protected:

    //! \brief The OpenGL whindows holding the context
    GLFWwindow *m_window = nullptr;
    float m_deltaTime = 0.0f;
    double m_lastTime = 0.0;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_fb_width;
    uint32_t m_fb_height;
    uint32_t m_mouse_x;
    uint32_t m_mouse_y;
    float m_scroll_x = 0.0f;
    float m_scroll_y = 0.0f;
    std::string m_title;
};

#endif
