#include "Application.hpp"
#include <cassert>
#include <iostream>

//------------------------------------------------------------------------------
//! \brief Callback when the OpenGL base window has been resized. Dispatch this
//! event to all BrowserView.
//------------------------------------------------------------------------------
void reshape_callback(GLFWwindow* ptr, int width, int height)
{
    assert(nullptr != ptr);
    Application* window = static_cast<Application*>(glfwGetWindowUserPointer(ptr));
    window->SetFBSize(width, height);
}

//------------------------------------------------------------------------------
//! \brief Callback when the mouse has clicked inside the OpenGL base window.
//! Dispatch this event to all BrowserView.
//------------------------------------------------------------------------------
void mouse_callback(GLFWwindow* /*ptr*/, int /*btn*/, int /*state*/, int /*mods*/)
{
    //assert(nullptr != ptr);
    //Application* window = static_cast<Application*>(glfwGetWindowUserPointer(ptr));
}

//------------------------------------------------------------------------------
//! \brief Callback when the mouse has scrolled inside the OpenGL base window.
//! Dispatch this event to all BrowserView.
//------------------------------------------------------------------------------
void scroll_callback(GLFWwindow* ptr, double xoffset, double yoffset)
{
    // Get the window.
    assert(nullptr != ptr);
    GLWindow* window = static_cast<GLWindow*>(glfwGetWindowUserPointer(ptr));
    window->SetMouseScroll(xoffset, yoffset);
}

//------------------------------------------------------------------------------
//! \brief Callback when the mouse has been displaced inside the OpenGL base
//! window. Dispatch this event to all BrowserView.
//------------------------------------------------------------------------------
void motion_callback(GLFWwindow* ptr, double x, double y)
{
    assert(nullptr != ptr);
    Application* window = static_cast<Application*>(glfwGetWindowUserPointer(ptr));
    window->SetMousePos((uint32_t) x, (uint32_t) y);
}

//------------------------------------------------------------------------------
//! \brief Callback when the keybaord has been pressed inside the OpenGL base
//! window. Dispatch this event to all BrowserView.
//------------------------------------------------------------------------------
void keyboard_callback(GLFWwindow *ptr, int key, int scancode,
                       int /*action*/, int /*mods*/)
{
    assert(nullptr != ptr);
    Application* window = static_cast<Application*>(glfwGetWindowUserPointer(ptr));
    printf("key: %d, scancode: %d\n", key, scancode);
    switch (key) {
        case 256:
            window->exit();
    }
}
