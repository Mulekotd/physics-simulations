#pragma once

#include <memory>
#include <optional>

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/trigonometric.hpp>

#include "app/Constants.hpp"
#include "engine/Camera2D.hpp"
#include "engine/Camera3D.hpp"
#include "engine/ShaderProgram.hpp"
#include "engine/TextureManager.hpp"
#include "physics/Field.hpp"
#include "physics/Motion.hpp"
#include "ui/InputManager.hpp"

namespace Application {
    enum class CameraMode {
        TwoD,
        ThreeD
    };

    struct DragState {
        bool active = false;
        glm::vec3 planePoint{ 0.f, 0.f, 0.f };
        glm::vec3 planeNormal{ 0.f, 0.f, 1.f };
        glm::vec3 lastPosition{ 0.f, 0.f, 0.f };
        std::optional<std::size_t> index;
    };

    struct RuntimeState {
        int count = 250;
        bool isRunning = false;
    };

    inline RuntimeState context;
    
    inline glm::ivec2 resolution = { Constants::Window::DEFAULT_WIDTH, Constants::Window::DEFAULT_HEIGHT };

    inline GLFWwindow* window = nullptr;
    inline InputManager& input = InputManager::Instance();

    inline Field world = Field{ glm::vec2{ resolution } };
    inline Camera2D camera = Camera2D{ world };
    inline Camera3D camera3D = Camera3D{ glm::vec3{ 0.f, 0.f, 600.f } };
    inline CameraMode cameraMode = CameraMode::TwoD;

    inline ShaderProgram particleShader;
    inline TextureManager textureManager;
    inline TextureId globalParticleTexture = 0;
    inline int globalTextureIndex = 0;
    inline int selectedTextureIndex = 0;

    inline std::unique_ptr<Simulation::Motion> motion = nullptr;
    inline std::optional<std::size_t> selectedParticle;
    inline bool showParticleWindow = false;
    inline DragState drag;

    bool Init();
    void Update(float dt);
    void Render();
    void Tick(float dt);
    void Cleanup();

    glm::vec3 ProjectToNDC(const glm::vec3& position);
    glm::vec3 ParticleRadiusAxis();
    bool IsParticleVisible(const glm::vec3& position, float radius);
}
