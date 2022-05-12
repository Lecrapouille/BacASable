#include "Application.hpp"
#include <iostream>

//------------------------------------------------------------------------------
Application::Application(uint32_t const width, uint32_t const height, const char *title)
    : GLWindow(width, height, title), stop(false)
{}

//------------------------------------------------------------------------------
Application::~Application()
{
    // Free GPU memory
    GLCore::deleteProgram(m_prog);
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

//------------------------------------------------------------------------------
void Application::exit()
{
    stop = true;
}

//------------------------------------------------------------------------------
bool Application::setup()
{
    // Windows events
    GLCHECK(glfwSetFramebufferSizeCallback(m_window, reshape_callback));
    GLCHECK(glfwSetKeyCallback(m_window, keyboard_callback));
    GLCHECK(glfwSetCursorPosCallback(m_window, motion_callback));
    GLCHECK(glfwSetMouseButtonCallback(m_window, mouse_callback));
    GLCHECK(glfwSetScrollCallback(m_window, scroll_callback));

    // Compile vertex and fragment shaders
    m_prog = GLCore::createShaderProgram("src/Shaders/points.vert",
                                         "src/Shaders/points.frag");
    if (m_prog == 0)
    {
        std::cerr << "shader compile failed" << std::endl;
        return false;
    }

    // Get locations of shader variables (attributes and uniforms)
    m_locations[POSITION] = GLCHECK(glGetAttribLocation(m_prog, "position"));
    m_locations[RESOLUTION] = GLCHECK(glGetUniformLocation(m_prog, "iResolution"));
    m_locations[MOUSE] = GLCHECK(glGetUniformLocation(m_prog, "iMouse"));
    m_locations[TIME] = GLCHECK(glGetUniformLocation(m_prog, "iTime"));
    m_locations[SCROLL] = GLCHECK(glGetUniformLocation(m_prog, "iScroll"));

    GLfloat vertices[] = {
        -1.0f,  1.0f,       // left     top
         1.0f, -1.0f,       // right    bottom
        -1.0f, -1.0f,       // left     bottom
         1.0f,  1.0f,       // right    top
    };

    GLuint vCount = 4 * 2;

    GLint indices[] = {
        0, 1, 2,
        0, 3, 1,
    };
    GLuint iCount = 2 * 3;

    GLCHECK(glGenVertexArrays(1, &m_vao));
    GLCHECK(glBindVertexArray(m_vao));
    GLCHECK(glGenBuffers(1, &m_vbo));
    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    GLCHECK(glBufferData(GL_ARRAY_BUFFER, vCount * sizeof(float),
                         vertices, GL_STATIC_DRAW));

    GLCHECK(glGenBuffers(1, &m_ebo));
    GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo));
    GLCHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, iCount * sizeof(GLuint),
                         indices, GL_STATIC_DRAW));

    GLCHECK(glEnableVertexAttribArray(m_locations[POSITION]));
    GLCHECK(glVertexAttribPointer(m_locations[POSITION], 2, GL_FLOAT, GL_FALSE, 0, 0));

    GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCHECK(glBindVertexArray(0));
    GLCHECK(glUseProgram(0));

    return true;
}

//------------------------------------------------------------------------------
bool Application::update()
{
    static float time = 0.0f;
    m_zoom += (m_scroll_y / 10.0f);
    std::cout << "Zoom: " << m_zoom << ", " << m_scroll_y << std::endl;
    m_scroll_x = m_scroll_y = 0.0f;

    // GLCHECK(glViewport(0, 0, 800, 800));

    GLCHECK(glClearColor(0.0f, 0.0f, 0.4f, 0.0f));
    GLCHECK(glClear(GL_COLOR_BUFFER_BIT));

    GLCHECK(glUseProgram(m_prog));
    GLCHECK(glBindVertexArray(m_vao));

    time += dt();
    GLCHECK(glUniform1f(m_locations[TIME], time));
    GLCHECK(glUniform2f(m_locations[RESOLUTION], float(m_fb_width), float(m_fb_height)));
    GLCHECK(glUniform2f(m_locations[MOUSE], float(m_mouse_x), float(m_mouse_y)));
    GLCHECK(glUniform1f(m_locations[SCROLL], m_zoom));

    GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo));
    GLCHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GLCHECK(glBindVertexArray(0));
    GLCHECK(glUseProgram(0));

    return !stop;
}

//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    Application win(800, 800, "Syracuse");
    int res = win.start();

    return res;
}
