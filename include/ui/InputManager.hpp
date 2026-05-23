#pragma once

#include <array>

#include <GLFW/glfw3.h>

#include "app/Types.hpp"

class InputManager {
public:
    static InputManager& Instance();

    // Query helpers
    [[nodiscard]] bool isKeyPressed(std::int32_t key) const noexcept;
    [[nodiscard]] bool isMouseButton(std::int32_t btn) const noexcept;
    [[nodiscard]] bool wasMousePressed(std::int32_t btn) noexcept;

    void cursorPos(double& x, double& y) const noexcept { x = m_mouseX; y = m_mouseY; }
    void consumeMouseDelta(double& dx, double& dy) noexcept;
    double consumeScrollDelta() noexcept;

    // GLFW callbacks
    static void CursorCallback(GLFWwindow* window, double xpos, double ypos);
    static void FramebufferSizeCallback(GLFWwindow* window, std::int32_t width, std::int32_t height);
    static void WindowPosCallback(GLFWwindow* window, std::int32_t xpos, std::int32_t ypos);
    static void MouseCallback(GLFWwindow* window, std::int32_t button, std::int32_t action, std::int32_t mods);
    static void KeyCallback(GLFWwindow* window, std::int32_t key, std::int32_t scancode, std::int32_t action, std::int32_t mods);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
    InputManager() = default;
    ~InputManager() = default;
    InputManager(const InputManager&) = delete;
    
    InputManager& operator=(const InputManager&) = delete;

    std::array<bool, 1024> m_keys  {false};
    std::array<bool, 16>   m_mouse {false};
    std::array<bool, 16>   m_mousePressed {false};

    double m_mouseX = 0.0, m_mouseY = 0.0;
    double m_prevX = 0.0, m_prevY = 0.0;
    double m_mouseDeltaX = 0.0, m_mouseDeltaY = 0.0;
    double m_scrollDeltaY = 0.0;
    std::int32_t    m_windowX = 0, m_windowY = 0;
    bool   m_hasWindowPosition = false;
    bool   m_panning = false;
};
