#pragma once

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace ImGuiLayer {

    void Init(GLFWwindow* window);
    void BeginFrame();
    void EndFrame();
    void Shutdown();
}
