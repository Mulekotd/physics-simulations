#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "backends/imgui_impl_glfw.h" 

#include "app/Application.hpp"
#include "ui/InputManager.hpp"

namespace {
    inline InputManager& input = InputManager::Instance();
}

InputManager& InputManager::Instance()
{
    static InputManager s;
    return s;
}

bool InputManager::isKeyPressed(int key) const noexcept
{
    return key >=0 && key < static_cast<int>(m_keys.size()) && m_keys[key];
}

bool InputManager::isMouseButton(int btn) const noexcept
{
    return btn >=0 && btn < static_cast<int>(m_mouse.size()) && m_mouse[btn];
}

bool InputManager::wasMousePressed(int btn) noexcept
{
    if (btn < 0 || btn >= static_cast<int>(m_mousePressed.size()))
        return false;
    
    bool pressed = m_mousePressed[btn];
    m_mousePressed[btn] = false;

    return pressed;
}

void InputManager::consumeMouseDelta(double& dx, double& dy) noexcept
{
    dx = m_mouseDeltaX;
    dy = m_mouseDeltaY;

    m_mouseDeltaX = 0.0;
    m_mouseDeltaY = 0.0;
}

double InputManager::consumeScrollDelta() noexcept
{
    double delta = m_scrollDeltaY;
    m_scrollDeltaY = 0.0;

    return delta;
}

/* ---------- GLFW Callbacks ---------------------------------------------- */

void InputManager::CursorCallback(GLFWwindow* window, double xpos, double ypos)
{
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    double deltaX = xpos - input.m_mouseX;
    double deltaY = ypos - input.m_mouseY;

    input.m_mouseDeltaX += deltaX;
    input.m_mouseDeltaY += deltaY;

    if (Application::cameraMode == Application::CameraMode::TwoD &&
        input.m_panning && !ImGui::GetIO().WantCaptureMouse)
    {
        double dx = xpos - input.m_prevX;
        double dy = ypos - input.m_prevY;

        input.m_prevX = xpos;
        input.m_prevY = ypos;

        // pixels → world units
        if (Application::resolution.x == 0 || Application::resolution.y == 0)
            return;

        const glm::vec2& size = Application::world.getSize();

        float worldDX = static_cast<float>(-dx) * (size.x / static_cast<float>(Application::resolution.x)) * Application::camera.getZoom();
        float worldDY = static_cast<float>(dy)  * (size.y / static_cast<float>(Application::resolution.y)) * Application::camera.getZoom();

        Application::camera.move({ worldDX, worldDY, 0.f });
    }

    input.m_mouseX = xpos;
    input.m_mouseY = ypos;
}

void InputManager::FramebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);

    Application::resolution = { width, height };
    Application::world.setSize(glm::vec2(width, height));
    Application::world.setDepth(Application::cameraMode == Application::CameraMode::ThreeD
                                    ? Constants::Simulation::FIELD_DEPTH_3D
                                    : Constants::Simulation::FIELD_DEPTH_2D);
    Application::world.setPosition(glm::vec3(Application::world.getSize() * 0.5f, 0.f));
}

void InputManager::WindowPosCallback(GLFWwindow*, int xpos, int ypos)
{
    if (!input.m_hasWindowPosition)
    {
        input.m_windowX = xpos;
        input.m_windowY = ypos;
        input.m_hasWindowPosition = true;
        return;
    }

    glm::vec2 delta{ static_cast<float>(xpos - input.m_windowX),
                     static_cast<float>(ypos - input.m_windowY) };

    input.m_windowX = xpos;
    input.m_windowY = ypos;

    if (Application::context.simulationMode == Simulation::Mode::FreeFall &&
        glm::dot(delta, delta) > 0.0f)
    {
        Application::AddWindowShake(delta);
    }
}

void InputManager::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

    if (key >= 0 && key < static_cast<int>(input.m_keys.size()))
        input.m_keys[key] = (action != GLFW_RELEASE);
}

void InputManager::MouseCallback(GLFWwindow* window, int button, int action, int mods)
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    const bool capturedByImGui = ImGui::GetIO().WantCaptureMouse;

    if (button >= 0 && button < static_cast<int>(input.m_mouse.size()))
        input.m_mouse[button] = (action != GLFW_RELEASE);

    if (button == GLFW_MOUSE_BUTTON_RIGHT && !capturedByImGui)
    {
        input.m_panning = (action == GLFW_PRESS);

        if (input.m_panning) 
            glfwGetCursorPos(window, &input.m_prevX, &input.m_prevY);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        input.m_panning = false;
    }

    if (!capturedByImGui &&
        action == GLFW_PRESS &&
        button >= 0 &&
        button < static_cast<int>(input.m_mousePressed.size()))
    {
        input.m_mousePressed[button] = true;
    }
}

void InputManager::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    constexpr float factor = 1.1f;

    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

    if (ImGui::GetIO().WantCaptureMouse)
        return;

    input.m_scrollDeltaY += yoffset;

    if (Application::cameraMode == Application::CameraMode::ThreeD &&
        (Application::drag.active || Application::lightDrag.active))
    {
        return;
    }

    if (yoffset > 0)
        Application::camera.setZoom(factor);
    else if (yoffset < 0)
        Application::camera.setZoom(1.0f / factor); 
}
