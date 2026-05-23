#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
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
        glm::vec3 lastPosition{ 0.f, 0.f, 0.f };
        glm::vec3 targetPosition{ 0.f, 0.f, 0.f };
        float32_t depth = 0.f;
        std::optional<std::size_t> index;
    };

    struct RuntimeState {
        std::int32_t count = 250;
        bool isRunning = false;
        bool paused = false;
        bool antiAliasing = true;
        Simulation::Mode simulationMode = Simulation::Mode::FreeFall;
        Simulation::Mode pendingSimulationMode = Simulation::Mode::FreeFall;
        Field::Properties pendingProperties{
            Constants::Physics::GRAVITY,
            Constants::Physics::DYNAMIC_FRICTION_COEFFICIENT,
            Constants::Physics::RESTITUTION_COEFFICIENT
        };
    };

    struct PointLight {
        glm::vec3 position{ 0.f, 0.f, 0.f };
        float32_t radius = 12.f;
        float32_t range = 420.f;
        float32_t intensity = 0.45f;
        std::uint32_t id = 0;
    };

    struct LightDragState {
        bool active = false;
        float32_t depth = 0.f;
        std::optional<std::size_t> index;
    };

    inline RuntimeState context;
    
    inline glm::ivec2 resolution = { Constants::Window::DEFAULT_WIDTH, Constants::Window::DEFAULT_HEIGHT };

    inline GLFWwindow* window = nullptr;
    inline InputManager& input = InputManager::Instance();

    inline Field world = Field{ glm::vec2{ resolution } };
    inline Camera2D camera = Camera2D{ world };
    inline Camera3D camera3D = Camera3D{ glm::vec3{ 0.f, 0.f, Constants::Camera::DEFAULT_3D_START_DISTANCE } };
    inline CameraMode cameraMode = CameraMode::TwoD;

    inline ShaderProgram particleShader;
    inline ShaderProgram shadowShader;
    inline TextureManager textureManager;
    inline TextureId globalParticleTexture = 0;
    inline std::int32_t globalTextureIndex = 0;
    inline std::int32_t selectedTextureIndex = 0;

    inline std::unique_ptr<Simulation::Motion> motion = nullptr;
    inline std::optional<std::size_t> selectedParticle;
    inline std::optional<std::size_t> selectedLight;
    inline bool showParticleWindow = false;
    inline bool showLightWindow = false;
    inline DragState drag;
    inline std::vector<PointLight> lights;
    inline LightDragState lightDrag;
    inline std::uint32_t nextLightId = 1;
    inline glm::vec3 windowShakeAcceleration{ 0.f, 0.f, 0.f };
    inline GLuint shadowFramebuffer = 0;
    inline GLuint shadowDepthTexture = 0;
    inline glm::mat4 shadowLightSpaceMatrix{ 1.f };
    inline bool shadowMapReady = false;

    bool Init();
    void Update(float32_t dt);
    void Render();
    void Tick(float32_t dt);
    void Cleanup();

    glm::vec3 ProjectToNDC(const glm::vec3& position);
    glm::vec2 ParticleRadiusNDC(const glm::vec3& position, float32_t radius);
    float32_t DepthCue(const glm::vec3& position);
    glm::vec3 ParticleLightDirection(const glm::vec3& position);
    bool IsAntiAliasingEnabled();
    bool IsParticleVisible(const glm::vec3& position, float32_t radius);
    void AddWindowShake(glm::vec2 delta);
}
