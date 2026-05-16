#include <algorithm>
#include <cstdio>

#include <glm/geometric.hpp>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ui/ImGuiLayer.hpp"
#include "ui/InputManager.hpp"

#include "app/Application.hpp"

namespace Application {

    bool Init()
    {
        if (!glfwInit())
            return false;

        window = glfwCreateWindow(resolution.x,
                                  resolution.y,
                                  Constants::Window::DEFAULT_TITLE,
                                  nullptr,
                                  nullptr);

        if (!window)
        {
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window);
        
        ImGuiLayer::Init(window);

        // GLFW callbacks setup
        glfwSetCursorPosCallback(window, InputManager::CursorCallback);
        glfwSetFramebufferSizeCallback(window, InputManager::FramebufferSizeCallback);
        glfwSetKeyCallback(window, InputManager::KeyCallback);
        glfwSetMouseButtonCallback(window, InputManager::MouseCallback);
        glfwSetScrollCallback(window, InputManager::ScrollCallback);

        glViewport(0, 0, resolution.x, resolution.y);

        return true;
    }

    void Update(float dt)
    {
        float frameDt = std::min(dt, Constants::Physics::MAX_FRAME_TIME);

        if (cameraMode == CameraMode::ThreeD)
        {
            glm::vec3 forward = camera3D.getForward();
            glm::vec3 right = camera3D.getRight();
            glm::vec3 move{ 0.f, 0.f, 0.f };

            if (input.isKeyPressed(GLFW_KEY_W)) move += forward;
            if (input.isKeyPressed(GLFW_KEY_S)) move -= forward;
            if (input.isKeyPressed(GLFW_KEY_A)) move -= right;
            if (input.isKeyPressed(GLFW_KEY_D)) move += right;
            if (input.isKeyPressed(GLFW_KEY_Q)) move -= glm::vec3{ 0.f, 1.f, 0.f };
            if (input.isKeyPressed(GLFW_KEY_E)) move += glm::vec3{ 0.f, 1.f, 0.f };

            if (glm::length(move) > 0.0f)
            {
                move = glm::normalize(move);
                camera3D.move(move * camera3D.getMoveSpeed() * frameDt);
            }

            double dx = 0.0, dy = 0.0;
            input.consumeMouseDelta(dx, dy);

            if (input.isMouseButton(GLFW_MOUSE_BUTTON_RIGHT) && !ImGui::GetIO().WantCaptureMouse)
                camera3D.processMouseDelta(static_cast<float>(dx), static_cast<float>(dy));
        } else
        {
            double dx = 0.0, dy = 0.0;
            input.consumeMouseDelta(dx, dy);
        }

        if (motion && input.wasMousePressed(GLFW_MOUSE_BUTTON_LEFT) && !ImGui::GetIO().WantCaptureMouse)
        {
            double mouseX = 0.0, mouseY = 0.0;
            input.cursorPos(mouseX, mouseY);

            if (cameraMode == CameraMode::ThreeD)
            {
                Ray ray = camera3D.screenPointToRay(mouseX, mouseY, glm::vec2(resolution));
                selectedParticle = motion->pickParticle(ray.origin, ray.direction);
            } else
            {
                float ndcX = (2.f * static_cast<float>(mouseX) / resolution.x) - 1.f;
                float ndcY = 1.f - (2.f * static_cast<float>(mouseY) / resolution.y);

                glm::vec3 worldPoint = camera.ndcToWorld({ ndcX, ndcY, 0.f });
                selectedParticle = motion->pickParticle2D(worldPoint);
            }

            if (selectedParticle.has_value())
            {
                showParticleWindow = true;

                if (cameraMode == CameraMode::ThreeD)
                {
                    Particle* particle = motion->getParticleMutable(*selectedParticle);

                    if (particle)
                    {
                        drag.active = true;
                        drag.index = selectedParticle;
                        drag.planePoint = particle->getPosition();
                        drag.planeNormal = camera3D.getForward();
                        drag.lastPosition = drag.planePoint;
                    }
                }
            }
        }

        if (drag.active && drag.index.has_value())
        {
            if (!input.isMouseButton(GLFW_MOUSE_BUTTON_LEFT))
            {
                drag.active = false;
                drag.index.reset();
            } else if (cameraMode == CameraMode::ThreeD)
            {
                double mouseX = 0.0, mouseY = 0.0;
                input.cursorPos(mouseX, mouseY);

                Ray ray = camera3D.screenPointToRay(mouseX, mouseY, glm::vec2(resolution));
                float denom = glm::dot(drag.planeNormal, ray.direction);

                if (std::fabs(denom) > Constants::Math::EPSILON)
                {
                    float t = glm::dot(drag.planePoint - ray.origin, drag.planeNormal) / denom;

                    if (t >= 0.0f)
                    {
                        glm::vec3 newPos = ray.origin + ray.direction * t;                        
                        Particle* particle = motion->getParticleMutable(*drag.index);

                        if (particle)
                        {
                            float dragDt = std::max(frameDt, Constants::Physics::MIN_DT);
                            glm::vec3 velocity = (newPos - drag.lastPosition) / dragDt;

                            float speed = glm::length(velocity);
                            if (speed > Constants::Physics::MAX_DRAG_SPEED) {
                                velocity = (velocity / speed) * Constants::Physics::MAX_DRAG_SPEED;
                            }

                            particle->setPosition(newPos);
                            particle->setVelocity(velocity);

                            drag.lastPosition = newPos;
                        }
                    }
                }
            }
        }

        if (motion)
        {
            static float accumulator = 0.0f;
            accumulator += frameDt;

            const float step = Constants::Physics::FIXED_TIME_STEP;

            while (accumulator >= step)
            {
                motion->update(step);
                accumulator -= step;
            }
        }
    }

    void Render() {
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (motion) motion->render();

        // ImGui rendering
        ImGuiLayer::BeginFrame();

        ImGui::SetNextWindowSize(ImVec2(350, 150), ImGuiCond_FirstUseEver);
        ImGui::Begin("Simulation Control");

        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        ImGui::SliderInt("Particles",
                 &context.count,
                 Constants::Simulation::PARTICLE_COUNT_MIN,
                 Constants::Simulation::PARTICLE_COUNT_MAX);

        int mode = (cameraMode == CameraMode::TwoD) ? 0 : 1;

        ImGui::RadioButton("Camera 2D", &mode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Camera 3D", &mode, 1);

        cameraMode = (mode == 0) ? CameraMode::TwoD : CameraMode::ThreeD;

        if (ImGui::Button(context.isRunning ? "Restart" : "Start"))
        {
            context.isRunning = true;

            world.setSize(glm::vec2(resolution));
            world.setPosition(glm::vec3(world.getHalfSize(), 0.f));

            camera.reset();
            camera3D = Camera3D{ glm::vec3{ world.getHalfSize(), 600.f } };

            selectedParticle.reset();
            showParticleWindow = false;
            drag.active = false;
            drag.index.reset();

            motion = std::make_unique<Simulation::Motion>(context.count, world);
        }

        ImGui::End();

        if (showParticleWindow && selectedParticle.has_value() && motion)
        {
            Particle* particle = motion->getParticleMutable(*selectedParticle);

            if (!particle)
            {
                showParticleWindow = false;
                selectedParticle.reset();
            } else
            {
                if (ImGui::Begin("Particle", &showParticleWindow))
                {
                    const glm::vec3& pos = particle->getPosition();
                    const glm::vec3& vel = particle->getVelocity();

                    ImGui::Text("Id: %u", particle->getId());
                    ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
                    ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", vel.x, vel.y, vel.z);
                    ImGui::Text("Mass: %.3f", particle->getMass());
                }

                ImGui::End();

                if (!showParticleWindow)
                {
                    selectedParticle.reset();
                    drag.active = false;
                    drag.index.reset();
                }
            }
        }

        ImGuiLayer::EndFrame();

        glfwSwapBuffers(window);
    }

    void Tick(float dt)
    {
        Update(dt);
        Render();
    }

    void Cleanup()
    {
        ImGuiLayer::Shutdown();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    glm::vec3 ProjectToNDC(const glm::vec3& position)
    {
        if (cameraMode == CameraMode::ThreeD)
            return camera3D.worldToNDC(position, glm::vec2(resolution));

        return camera.worldToNDC(position);
    }

    glm::vec3 ParticleRadiusAxis()
    {
        if (cameraMode == CameraMode::ThreeD)
            return camera3D.getRight();

        return { 1.f, 0.f, 0.f };
    }

    bool IsParticleVisible(const glm::vec3& position, float radius)
    {
        if (cameraMode == CameraMode::ThreeD)
            return camera3D.isSphereInFront(position, radius);

        glm::vec3 ndc = camera.worldToNDC(position);
        return ndc.x + radius >= -1.f && ndc.x - radius <= 1.f &&
               ndc.y + radius >= -1.f && ndc.y - radius <= 1.f;
    }
}
