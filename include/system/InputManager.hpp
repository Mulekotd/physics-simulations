#pragma once

#include <array>

#include <GLFW/glfw3.h>

class InputManager {
public:
    static InputManager& Instance();

    // Query helpers
    [[nodiscard]] bool isKeyPressed(int key) const noexcept;
    [[nodiscard]] bool isMouseButton(int btn) const noexcept;
    void cursorPos(double& x, double& y) const noexcept { x = m_mouseX; y = m_mouseY; }

    // GLFW callbacks
    static void CursorCallback(GLFWwindow* window, double xpos, double ypos);
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void MouseCallback(GLFWwindow* window, int button, int action, int mods);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
    InputManager() = default;
    ~InputManager() = default;

    InputManager(const InputManager&) = delete;
    
    InputManager& operator=(const InputManager&) = delete;

    std::array<bool, 1024> m_keys  {false};
    std::array<bool, 16>   m_mouse {false};
    double m_mouseX = 0.0, m_mouseY = 0.0;
    double m_prevX = 0.0, m_prevY = 0.0;
    bool   m_panning = false;
};
