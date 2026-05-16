#pragma once

#include <memory>

#include <GLFW/glfw3.h>

#include "system/InputManager.hpp"

#include "common/Constants.hpp"
#include "common/Context.hpp"
#include "common/Dimensions.hpp"

#include "core/Field.hpp"
#include "core/Camera2D.hpp"

#include "simulation/Motion.hpp"

namespace Application {
        
    inline RuntimeState context;
    
    inline Dimensions resolution = { Constants::Window::DEFAULT_WIDTH, Constants::Window::DEFAULT_HEIGHT };

    inline GLFWwindow* window = nullptr;
    inline InputManager& input = InputManager::Instance();

    inline Field world(resolution);
    inline Camera2D camera(world);

    inline std::unique_ptr<Simulation::Motion> motion = nullptr;

    bool Init();
    void Update(float dt);
    void Render();
    void Tick(float dt);
    void Cleanup();
}
