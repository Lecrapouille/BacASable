#include "Application.hpp"
#include <cassert>

//------------------------------------------------------------------------------
//! \brief Callback when the OpenGL base window has been resized. Dispatch this
//! event to all BrowserView.
//------------------------------------------------------------------------------
static void reshape_callback(GLFWwindow* /*ptr*/, int /*w*/, int /*h*/)
{
    //assert(nullptr != ptr);
    //Application* window = static_cast<Application*>(glfwGetWindowUserPointer(ptr));
}

//------------------------------------------------------------------------------
//! \brief Callback when the mouse has clicked inside the OpenGL base window.
//! Dispatch this event to all BrowserView.
//------------------------------------------------------------------------------
static void mouse_callback(GLFWwindow* ptr, int btn, int state, int /*mods*/)
{
    //assert(nullptr != ptr);
    //Application* window = static_cast<Application*>(glfwGetWindowUserPointer(ptr));
}

//------------------------------------------------------------------------------
//! \brief Callback when the mouse has been displaced inside the OpenGL base
//! window. Dispatch this event to all BrowserView.
//------------------------------------------------------------------------------
static void motion_callback(GLFWwindow* ptr, double x, double y)
{
    //assert(nullptr != ptr);
    //Application* window = static_cast<Application*>(glfwGetWindowUserPointer(ptr));
}

//------------------------------------------------------------------------------
//! \brief Callback when the keybaord has been pressed inside the OpenGL base
//! window. Dispatch this event to all BrowserView.
//------------------------------------------------------------------------------
static void keyboard_callback(GLFWwindow* ptr, int key, int /*scancode*/,
                              int action, int /*mods*/)
{
    //assert(nullptr != ptr);
    Application* window = static_cast<Application*>(glfwGetWindowUserPointer(ptr));
}
