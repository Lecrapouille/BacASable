#ifndef APPLICATION_HPP
#  define APPLICATION_HPP

#  include "GLWindow.hpp"
#  include "GLCore.hpp"

void reshape_callback(GLFWwindow* ptr, int w, int h);
void mouse_callback(GLFWwindow* ptr, int btn, int state, int mods);
void motion_callback(GLFWwindow* ptr, double x, double y);
void keyboard_callback(GLFWwindow* ptr, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* ptr, double xoffset, double yoffset);

// ****************************************************************************
//! \brief Extend the OpenGL base window and add Chromium Embedded Framework
//! browser views.
// ****************************************************************************
class Application: public GLWindow
{
public:

    //! \brief Default construction: define the window dimension and title
    Application(uint32_t const width, uint32_t const height, const char *title);

    //! \brief Destructor
    ~Application();

    void exit();

private:
    bool stop;

private: // Concrete implementation from GLWindow

    virtual bool setup() override;
    virtual bool update() override;

private: // OpenGL Syracuse

    //! \brief OpenGL shader program handle
    GLuint m_prog = 0;
    //! \brief OpenGL vertex array object handle
    GLuint m_vao = 0;
    //! \brief OpenGL vertex buffer object handle
    GLuint m_vbo = 0;
    //! \brief OpenGL element buffer object handle
    GLuint m_ebo = 0;

    //! \brief OpenGL shader variable locations
    enum Locations { POSITION, RESOLUTION, TIME, MOUSE, SCROLL, _MAX };
    GLint m_locations[_MAX];
    float m_zoom = 0.0f;
};

#endif // APPLICATION_HPP
