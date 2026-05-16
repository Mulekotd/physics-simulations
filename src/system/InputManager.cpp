#include "backends/imgui_impl_glfw.h" 

#include "common/Application.hpp"
#include "common/Dimensions.hpp"

#include "system/InputManager.hpp"

namespace {
    inline InputManager& input = InputManager::Instance();
}

/* ---------- Singleton --------------------------------------------------- */

InputManager& InputManager::Instance() {
    static InputManager s;
    return s;
}

/* ---------- Query helpers ----------------------------------------------- */

bool InputManager::isKeyPressed(int key) const noexcept {
    return key >=0 && key < static_cast<int>(m_keys.size()) && m_keys[key];
}

bool InputManager::isMouseButton(int btn) const noexcept {
    return btn >=0 && btn < static_cast<int>(m_mouse.size()) && m_mouse[btn];
}

/* ---------- GLFW Callbacks ---------------------------------------------- */

void InputManager::CursorCallback(GLFWwindow* window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    if (input.m_panning && !ImGui::GetIO().WantCaptureMouse) {
        double dx = xpos - input.m_prevX;
        double dy = ypos - input.m_prevY;

        input.m_prevX = xpos;
        input.m_prevY = ypos;

        // pixels → world units
        const Dimensions& size = Application::world.getSize();

        float worldDX = static_cast<float>(-dx) * (size.width  / Application::resolution.width ) * Application::camera.getZoom();
        float worldDY = static_cast<float>( dy) * (size.height / Application::resolution.height) * Application::camera.getZoom();

        Application::camera.move({ worldDX, worldDY, 0.f });
    }

    input.m_mouseX = xpos;
    input.m_mouseY = ypos;
}

void InputManager::FramebufferSizeCallback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);

    Application::resolution = { width, height };
    Application::world.setSize(Application::resolution);
    Application::world.setPosition(Application::resolution.centerAsVector());
}

void InputManager::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

    if (key >= 0 && key < static_cast<int>(input.m_keys.size()))
        input.m_keys[key] = (action != GLFW_RELEASE);
}

void InputManager::MouseCallback(GLFWwindow* window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    if (ImGui::GetIO().WantCaptureMouse) return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        input.m_panning = (action == GLFW_PRESS);

        if (input.m_panning) 
            glfwGetCursorPos(window, &input.m_prevX, &input.m_prevY);
    }

    if (button >= 0 && button < static_cast<int>(input.m_mouse.size()))
        input.m_mouse[button] = (action != GLFW_RELEASE);
}

void InputManager::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    constexpr float factor = 1.1f;

    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

    if (ImGui::GetIO().WantCaptureMouse) return;

    if (yoffset > 0) { Application::camera.setZoom(factor); }
    else if (yoffset < 0) { Application::camera.setZoom(1.0f / factor); }
}
