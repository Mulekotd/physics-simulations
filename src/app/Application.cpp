#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <glm/geometric.hpp>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ui/ImGuiLayer.hpp"
#include "ui/InputManager.hpp"

#include "app/Application.hpp"

namespace Application {
    namespace {
        std::string readTextFile(const std::string& path)
        {
            std::ifstream file(path);

            if (!file)
                return {};

            std::ostringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }

        std::string readProjectTextFile(const std::string& path)
        {
            std::string contents = readTextFile(path);

            if (!contents.empty())
                return contents;

            return readTextFile("../" + path);
        }

        glm::vec3 mouseToFieldPoint(double mouseX, double mouseY)
        {
            if (cameraMode == CameraMode::ThreeD)
            {
                Ray ray = camera3D.screenPointToRay(mouseX, mouseY, glm::vec2(resolution));

                if (std::fabs(ray.direction.z) > Constants::Math::EPSILON)
                {
                    float t = -ray.origin.z / ray.direction.z;

                    if (t >= 0.f)
                        return ray.origin + ray.direction * t;
                }

                return ray.origin + ray.direction * 300.f;
            }

            float ndcX = (2.f * static_cast<float>(mouseX) / resolution.x) - 1.f;
            float ndcY = 1.f - (2.f * static_cast<float>(mouseY) / resolution.y);

            return camera.ndcToWorld({ ndcX, ndcY, 0.f });
        }

        std::optional<std::size_t> pickLight(const glm::vec3& worldPoint)
        {
            float bestDistSq = std::numeric_limits<float>::max();
            std::optional<std::size_t> bestIndex;

            for (std::size_t i = 0; i < lights.size(); ++i)
            {
                glm::vec3 delta = lights[i].position - worldPoint;
                float radius = lights[i].radius * 1.8f;
                float distSq = glm::dot(delta, delta);

                if (distSq <= radius * radius && distSq < bestDistSq)
                {
                    bestDistSq = distSq;
                    bestIndex = i;
                }
            }

            return bestIndex;
        }

        void drawCircleNdc(const glm::vec3& center, const glm::vec2& radius, float r, float g, float b, float a)
        {
            if (!std::isfinite(center.x) || !std::isfinite(center.y) ||
                !std::isfinite(radius.x) || !std::isfinite(radius.y) ||
                radius.x <= 0.f || radius.y <= 0.f) return;

            glColor4f(r, g, b, a);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(center.x, center.y);

            constexpr int segments = 36;
            for (int i = 0; i <= segments; ++i)
            {
                float angle = (2.f * Constants::Math::PI * i) / segments;
                glVertex2f(center.x + std::cos(angle) * radius.x,
                           center.y + std::sin(angle) * radius.y);
            }

            glEnd();
        }

        float lineDepthFade(const glm::vec3& a, const glm::vec3& b)
        {
            if (cameraMode != CameraMode::ThreeD)
                return 1.0f;

            float depthA = glm::dot(a - camera3D.getPosition(), camera3D.getForward());
            float depthB = glm::dot(b - camera3D.getPosition(), camera3D.getForward());
            float depth = (depthA + depthB) * 0.5f;
            float farDepth = std::max(world.getDepth() + 900.f, 1.0f);

            return std::clamp(1.0f - depth / farDepth, 0.16f, 0.78f);
        }

        void drawProjectedLine(const glm::vec3& a,
                               const glm::vec3& b,
                               float r,
                               float g,
                               float bl,
                               float alpha,
                               bool distort = false)
        {
            if (cameraMode == CameraMode::ThreeD &&
                !camera3D.isSphereInFront((a + b) * 0.5f, glm::length(b - a) * 0.5f))
            {
                return;
            }

            constexpr int segments = 18;
            glColor4f(r, g, bl, alpha * lineDepthFade(a, b));
            glBegin(GL_LINE_STRIP);

            for (int i = 0; i <= segments; ++i)
            {
                float t = static_cast<float>(i) / static_cast<float>(segments);
                glm::vec3 p = a + (b - a) * t;

                if (distort && cameraMode == CameraMode::ThreeD)
                {
                    float depth = glm::dot(p - camera3D.getPosition(), camera3D.getForward());
                    float distanceScale = std::clamp(depth / std::max(world.getDepth(), 1.0f), 0.0f, 1.0f);
                    float wave = std::sin(t * Constants::Math::PI * 6.0f + depth * 0.018f);
                    p += camera3D.getUp() * (wave * distanceScale * 4.5f);
                }

                glm::vec3 ndc = ProjectToNDC(p);
                if (std::isfinite(ndc.x) && std::isfinite(ndc.y))
                    glVertex2f(ndc.x, ndc.y);
            }

            glEnd();
        }

        std::vector<glm::vec3> clipAgainstCameraNearPlane(const std::vector<glm::vec3>& polygon)
        {
            if (cameraMode != CameraMode::ThreeD || polygon.empty())
                return polygon;

            constexpr float nearDepth = 2.0f;
            std::vector<glm::vec3> clipped;
            clipped.reserve(polygon.size() + 2);

            auto depthOf = [](const glm::vec3& point)
            {
                return glm::dot(point - camera3D.getPosition(), camera3D.getForward());
            };

            for (std::size_t i = 0; i < polygon.size(); ++i)
            {
                const glm::vec3& current = polygon[i];
                const glm::vec3& next = polygon[(i + 1) % polygon.size()];
                float currentDepth = depthOf(current);
                float nextDepth = depthOf(next);
                bool currentInside = currentDepth > nearDepth;
                bool nextInside = nextDepth > nearDepth;

                if (currentInside && nextInside)
                {
                    clipped.push_back(next);
                }
                else if (currentInside != nextInside)
                {
                    float t = (nearDepth - currentDepth) / (nextDepth - currentDepth);
                    clipped.push_back(current + (next - current) * t);

                    if (nextInside)
                        clipped.push_back(next);
                }
            }

            return clipped;
        }

        void drawProjectedFace(const std::vector<glm::vec3>& face, float r, float g, float b, float alpha)
        {
            std::vector<glm::vec3> clipped = clipAgainstCameraNearPlane(face);
            if (clipped.size() < 3)
                return;

            glColor4f(r, g, b, alpha);
            glBegin(GL_TRIANGLE_FAN);

            for (const glm::vec3& point : clipped)
            {
                glm::vec3 ndc = ProjectToNDC(point);
                if (std::isfinite(ndc.x) && std::isfinite(ndc.y))
                    glVertex2f(ndc.x, ndc.y);
            }

            glEnd();
        }

        void drawFreeFallBounds()
        {
            if (context.simulationMode != Simulation::Mode::FreeFall)
                return;

            particleShader.stop();
            glDisable(GL_TEXTURE_2D);

            glm::vec3 center = world.getPosition();
            glm::vec2 halfSize = world.getHalfSize();
            float halfDepth = world.getHalfDepth();

            glm::vec3 min{ center.x - halfSize.x, center.y - halfSize.y, center.z - halfDepth };
            glm::vec3 max{ center.x + halfSize.x, center.y + halfSize.y, center.z + halfDepth };

            if (cameraMode != CameraMode::ThreeD || halfDepth <= Constants::Math::EPSILON)
            {
                glm::vec3 bottomLeft{ min.x, min.y, center.z };
                glm::vec3 bottomRight{ max.x, min.y, center.z };
                glm::vec3 topRight{ max.x, max.y, center.z };
                glm::vec3 topLeft{ min.x, max.y, center.z };

                drawProjectedLine(bottomLeft, bottomRight, 0.38f, 0.64f, 0.9f, 0.5f);
                drawProjectedLine(bottomRight, topRight, 0.38f, 0.64f, 0.9f, 0.34f);
                drawProjectedLine(topRight, topLeft, 0.38f, 0.64f, 0.9f, 0.22f);
                drawProjectedLine(topLeft, bottomLeft, 0.38f, 0.64f, 0.9f, 0.34f);

                return;
            }

            const float x0 = min.x, x1 = max.x;
            const float y0 = min.y, y1 = max.y;
            const float z0 = min.z, z1 = max.z;

            glm::vec3 c000{ x0, y0, z0 };
            glm::vec3 c100{ x1, y0, z0 };
            glm::vec3 c010{ x0, y1, z0 };
            glm::vec3 c110{ x1, y1, z0 };
            glm::vec3 c001{ x0, y0, z1 };
            glm::vec3 c101{ x1, y0, z1 };
            glm::vec3 c011{ x0, y1, z1 };
            glm::vec3 c111{ x1, y1, z1 };

            drawProjectedFace({ c000, c100, c110, c010 }, 0.2f, 0.42f, 0.68f, 0.12f);
            drawProjectedFace({ c001, c101, c111, c011 }, 0.2f, 0.42f, 0.68f, 0.08f);
            drawProjectedFace({ c000, c001, c101, c100 }, 0.24f, 0.5f, 0.78f, 0.14f);
            drawProjectedFace({ c010, c011, c111, c110 }, 0.18f, 0.36f, 0.58f, 0.07f);
            drawProjectedFace({ c000, c001, c011, c010 }, 0.18f, 0.38f, 0.62f, 0.09f);
            drawProjectedFace({ c100, c101, c111, c110 }, 0.18f, 0.38f, 0.62f, 0.09f);
        }

        struct ShadowFace {
            glm::vec3 origin;
            glm::vec3 u;
            glm::vec3 v;
            glm::vec3 normal;
            float uLength;
            float vLength;
        };

        void drawParticleShadowOnFace(const Particle& particle, const PointLight& light, const ShadowFace& face)
        {
            glm::vec3 lightToParticle = particle.getPosition() - light.position;
            float particleDistance = glm::length(lightToParticle);

            if (particleDistance <= Constants::Math::EPSILON || particleDistance > light.range)
                return;

            float denom = glm::dot(lightToParticle, face.normal);
            if (std::abs(denom) <= Constants::Math::EPSILON)
                return;

            float t = glm::dot(face.origin - light.position, face.normal) / denom;
            if (t <= 1.0f)
                return;

            glm::vec3 hit = light.position + lightToParticle * t;
            glm::vec3 local = hit - face.origin;
            float uCenter = glm::dot(local, face.u);
            float vCenter = glm::dot(local, face.v);

            if (uCenter < 0.0f || uCenter > face.uLength || vCenter < 0.0f || vCenter > face.vLength)
                return;

            float fade = std::clamp(1.0f - particleDistance / light.range, 0.0f, 1.0f);
            float projectionScale = std::clamp(t, 1.0f, 3.6f);
            float shadowRadius = std::clamp(particle.getRadius() * projectionScale, 4.0f, 95.0f);
            float alpha = light.intensity * fade * fade * (0.28f / std::sqrt(projectionScale));

            std::vector<glm::vec3> shadow;
            shadow.reserve(28);

            constexpr int segments = 28;
            for (int i = 0; i < segments; ++i)
            {
                float angle = (2.0f * Constants::Math::PI * i) / static_cast<float>(segments);
                float u = std::clamp(uCenter + std::cos(angle) * shadowRadius, 0.0f, face.uLength);
                float v = std::clamp(vCenter + std::sin(angle) * shadowRadius, 0.0f, face.vLength);
                shadow.push_back(face.origin + face.u * u + face.v * v);
            }

            drawProjectedFace(shadow, 0.0f, 0.0f, 0.0f, alpha);
        }

        void drawFreeFallParticleShadows()
        {
            if (cameraMode != CameraMode::ThreeD ||
                context.simulationMode != Simulation::Mode::FreeFall ||
                !motion ||
                lights.empty())
            {
                return;
            }

            glm::vec3 center = world.getPosition();
            glm::vec2 halfSize = world.getHalfSize();
            float halfDepth = world.getHalfDepth();

            if (halfDepth <= Constants::Math::EPSILON)
                return;

            glm::vec3 min{ center.x - halfSize.x, center.y - halfSize.y, center.z - halfDepth };
            glm::vec3 max{ center.x + halfSize.x, center.y + halfSize.y, center.z + halfDepth };
            float width = max.x - min.x;
            float height = max.y - min.y;
            float depth = max.z - min.z;

            const ShadowFace faces[] = {
                { { min.x, min.y, min.z }, { 1.f, 0.f, 0.f }, { 0.f, 0.f, 1.f }, { 0.f, 1.f, 0.f }, width, depth },
                { { min.x, max.y, min.z }, { 1.f, 0.f, 0.f }, { 0.f, 0.f, 1.f }, { 0.f, -1.f, 0.f }, width, depth },
                { { min.x, min.y, min.z }, { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 0.f, 0.f, 1.f }, width, height },
                { { min.x, min.y, max.z }, { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 0.f, 0.f, -1.f }, width, height },
                { { min.x, min.y, min.z }, { 0.f, 0.f, 1.f }, { 0.f, 1.f, 0.f }, { 1.f, 0.f, 0.f }, depth, height },
                { { max.x, min.y, min.z }, { 0.f, 0.f, 1.f }, { 0.f, 1.f, 0.f }, { -1.f, 0.f, 0.f }, depth, height }
            };

            for (const auto& light : lights)
            {
                for (const Particle& particle : motion->particles())
                {
                    for (const ShadowFace& face : faces)
                        drawParticleShadowOnFace(particle, light, face);
                }
            }
        }

        void drawLighting()
        {
            if (!motion || lights.empty())
                return;

            particleShader.stop();
            glDisable(GL_TEXTURE_2D);

            drawFreeFallParticleShadows();

            for (const auto& light : lights)
            {
                if (!IsParticleVisible(light.position, light.radius))
                    continue;

                glm::vec3 ndc = ProjectToNDC(light.position);
                glm::vec2 ndcRadius = ParticleRadiusNDC(light.position, light.radius);
                drawCircleNdc(ndc, ndcRadius * 1.8f, 1.f, 0.86f, 0.35f, 0.18f);
                drawCircleNdc(ndc, ndcRadius, 1.f, 0.92f, 0.42f, 0.95f);
            }

            glColor4f(1.f, 1.f, 1.f, 1.f);
        }

        void updateFieldDepthForCameraMode()
        {
            world.setDepth(cameraMode == CameraMode::ThreeD
                               ? Constants::Simulation::FIELD_DEPTH_3D
                               : Constants::Simulation::FIELD_DEPTH_2D);
        }
    }

    bool Init()
    {
        if (!glfwInit())
            return false;

        glfwWindowHint(GLFW_SAMPLES, 4);

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
        glfwSetWindowPosCallback(window, InputManager::WindowPosCallback);
        glfwSetKeyCallback(window, InputManager::KeyCallback);
        glfwSetMouseButtonCallback(window, InputManager::MouseCallback);
        glfwSetScrollCallback(window, InputManager::ScrollCallback);

        glViewport(0, 0, resolution.x, resolution.y);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_MULTISAMPLE);
        updateFieldDepthForCameraMode();

        std::string vertexShader = readProjectTextFile("shaders/particle.vert");
        std::string fragmentShader = readProjectTextFile("shaders/particle.frag");

        if (vertexShader.empty() || fragmentShader.empty() ||
            !particleShader.init(vertexShader, fragmentShader))
        {
            std::fprintf(stderr, "Failed to initialize particle shader.\n");
            return false;
        }

        globalParticleTexture = textureManager.createSolid("White", 255, 255, 255, 255);

        return true;
    }

    void Update(float dt)
    {
        float frameDt = std::min(dt, Constants::Physics::MAX_FRAME_TIME);
        double scrollDelta = input.consumeScrollDelta();
        windowShakeAcceleration *= std::exp(-Constants::Physics::WINDOW_SHAKE_FORCE_DECAY * frameDt);

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
            glm::vec3 fieldPoint = mouseToFieldPoint(mouseX, mouseY);

            lightDrag.index = pickLight(fieldPoint);
            lightDrag.active = lightDrag.index.has_value();

            if (lightDrag.active && cameraMode == CameraMode::ThreeD && lightDrag.index.has_value())
            {
                const PointLight& light = lights[*lightDrag.index];
                lightDrag.depth = std::max(glm::dot(light.position - camera3D.getPosition(),
                                           camera3D.getForward()),
                                           Constants::Physics::DRAG_MIN_DEPTH);
            }

            if (!lightDrag.active && cameraMode == CameraMode::ThreeD)
            {
                Ray ray = camera3D.screenPointToRay(mouseX, mouseY, glm::vec2(resolution));
                selectedParticle = motion->pickParticle(ray.origin, ray.direction);
            } else if (!lightDrag.active)
            {
                selectedParticle = motion->pickParticle2D(fieldPoint);
            }

            if (!context.paused && !lightDrag.active && selectedParticle.has_value())
            {
                showParticleWindow = true;

                if (cameraMode == CameraMode::ThreeD)
                {
                    Particle* particle = motion->getParticleMutable(*selectedParticle);

                    if (particle)
                    {
                        drag.active = true;
                        drag.index = selectedParticle;
                        drag.lastPosition = particle->getPosition();
                        drag.targetPosition = particle->getPosition();
                        drag.depth = std::max(glm::dot(particle->getPosition() - camera3D.getPosition(),
                                              camera3D.getForward()),
                                              1.0f);
                        motion->setPinnedParticle(drag.index);
                    }
                }
            }
        }

        if (lightDrag.active && lightDrag.index.has_value())
        {
            if (!input.isMouseButton(GLFW_MOUSE_BUTTON_LEFT))
            {
                lightDrag.active = false;
                lightDrag.index.reset();
            }
            else if (*lightDrag.index < lights.size())
            {
                double mouseX = 0.0, mouseY = 0.0;
                input.cursorPos(mouseX, mouseY);

                if (cameraMode == CameraMode::ThreeD)
                {
                    if (std::abs(scrollDelta) > Constants::Math::EPSILON)
                    {
                        float maxDepth = world.getDepth() + glm::length(glm::vec2(world.getSize())) + 1200.f;
                        lightDrag.depth = std::clamp(lightDrag.depth -
                                                         static_cast<float>(scrollDelta) * Constants::Physics::DRAG_SCROLL_DEPTH_STEP,
                                                     Constants::Physics::DRAG_MIN_DEPTH,
                                                     maxDepth);
                    }

                    Ray ray = camera3D.screenPointToRay(mouseX, mouseY, glm::vec2(resolution));
                    lights[*lightDrag.index].position = ray.origin + ray.direction * lightDrag.depth;
                }
                else
                {
                    lights[*lightDrag.index].position = mouseToFieldPoint(mouseX, mouseY);
                }
            }
        }

        if (context.paused && drag.active)
        {
            if (motion)
                motion->setPinnedParticle(std::nullopt);

            drag.active = false;
            drag.index.reset();
        }

        if (!context.paused && drag.active && drag.index.has_value())
        {
            if (!input.isMouseButton(GLFW_MOUSE_BUTTON_LEFT))
            {
                if (motion)
                    motion->setPinnedParticle(std::nullopt);

                drag.active = false;
                drag.index.reset();
            } else if (cameraMode == CameraMode::ThreeD)
            {
                double mouseX = 0.0, mouseY = 0.0;
                input.cursorPos(mouseX, mouseY);

                if (std::abs(scrollDelta) > Constants::Math::EPSILON)
                {
                    float maxDepth = world.getDepth() + glm::length(glm::vec2(world.getSize())) + 1200.f;
                    drag.depth = std::clamp(drag.depth -
                                                static_cast<float>(scrollDelta) * Constants::Physics::DRAG_SCROLL_DEPTH_STEP,
                                            Constants::Physics::DRAG_MIN_DEPTH,
                                            maxDepth);
                }

                Ray ray = camera3D.screenPointToRay(mouseX, mouseY, glm::vec2(resolution));
                glm::vec3 newPos = ray.origin + ray.direction * drag.depth;
                Particle* particle = motion->getParticleMutable(*drag.index);

                if (particle)
                {
                    drag.targetPosition = newPos;

                    float dragDt = std::max(frameDt, Constants::Physics::MIN_DT);
                    float follow = 1.f - std::exp(-Constants::Physics::DRAG_FOLLOW_STIFFNESS * dragDt);
                    glm::vec3 smoothedPos = particle->getPosition() + (drag.targetPosition - particle->getPosition()) * follow;
                    glm::vec3 velocity = (smoothedPos - drag.lastPosition) / dragDt;

                    float speed = glm::length(velocity);
                    if (speed > Constants::Physics::MAX_DRAG_SPEED) {
                        velocity = (velocity / speed) * Constants::Physics::MAX_DRAG_SPEED;
                    }

                    particle->setPosition(smoothedPos);
                    particle->setVelocity(velocity);

                    drag.lastPosition = smoothedPos;
                }
            }
        }

        if (motion && !context.paused)
        {
            if (context.simulationMode == Simulation::Mode::FreeFall &&
                glm::dot(windowShakeAcceleration, windowShakeAcceleration) > Constants::Math::EPSILON)
            {
                motion->addTransientAcceleration(windowShakeAcceleration);
            }

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

        if (context.antiAliasing)
            glEnable(GL_MULTISAMPLE);
        else
            glDisable(GL_MULTISAMPLE);

        drawFreeFallBounds();
        drawLighting();
        if (motion) motion->render();

        // ImGui rendering
        ImGuiLayer::BeginFrame();

        ImGui::SetNextWindowSize(ImVec2(320, 260), ImGuiCond_FirstUseEver);
        ImGui::Begin("Controls");

        if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Checkbox("Anti-aliasing", &context.antiAliasing);
        }

        if (ImGui::CollapsingHeader("Parameters", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool lockSimulationConfig = context.isRunning && !context.paused;
            if (lockSimulationConfig)
                ImGui::BeginDisabled();

            ImGui::SliderInt("Particles",
                             &context.count,
                             Constants::Simulation::PARTICLE_COUNT_MIN,
                             Constants::Simulation::PARTICLE_COUNT_MAX);

            ImGui::SliderFloat("Gravity", &context.pendingProperties.gravity, 0.0f, 50.0f, "%.2f");
            ImGui::SliderFloat("Friction", &context.pendingProperties.friction, 0.0f, 1.0f, "%.3f");
            ImGui::SliderFloat("Elasticity", &context.pendingProperties.restitution, 0.0f, 1.0f, "%.2f");

            if (lockSimulationConfig)
                ImGui::EndDisabled();
        }

        if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int mode = (cameraMode == CameraMode::TwoD) ? 0 : 1;

            ImGui::RadioButton("Camera 2D", &mode, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Camera 3D", &mode, 1);

            cameraMode = (mode == 0) ? CameraMode::TwoD : CameraMode::ThreeD;
            updateFieldDepthForCameraMode();

            bool lockSimulationConfig = context.isRunning && !context.paused;
            int simMode = (context.pendingSimulationMode == Simulation::Mode::FreeFall) ? 0 : 1;

            if (lockSimulationConfig)
                ImGui::BeginDisabled();

            ImGui::RadioButton("Free Fall", &simMode, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Orbit", &simMode, 1);
            context.pendingSimulationMode = (simMode == 0) ? Simulation::Mode::FreeFall : Simulation::Mode::Orbit;

            if (lockSimulationConfig)
                ImGui::EndDisabled();

            if (ImGui::Button("Add Light") && lights.size() < Constants::Lighting::MAX_LIGHTS)
            {
                lights.push_back({ world.getPosition(),
                                   Constants::Lighting::DEFAULT_RADIUS,
                                   Constants::Lighting::DEFAULT_RANGE,
                                   Constants::Lighting::DEFAULT_INTENSITY });
            }

            ImGui::SameLine();
            if (ImGui::Button("Clear Lights"))
            {
                lights.clear();
                lightDrag.active = false;
                lightDrag.index.reset();
            }

            if (ImGui::Button(context.isRunning ? "Restart" : "Start"))
            {
                context.isRunning = true;
                context.paused = false;
                context.simulationMode = context.pendingSimulationMode;

                world.setSize(glm::vec2(resolution));
                updateFieldDepthForCameraMode();
                world.setPosition(glm::vec3(world.getHalfSize(), 0.f));
                world.setProperties(context.pendingProperties);

                camera.reset();
                camera3D = Camera3D{ glm::vec3{ world.getHalfSize(), Constants::Camera::DEFAULT_3D_START_DISTANCE } };

                selectedParticle.reset();
                showParticleWindow = false;
                drag.active = false;
                drag.index.reset();
                lightDrag.active = false;
                lightDrag.index.reset();

                motion = std::make_unique<Simulation::Motion>(context.count, world, context.simulationMode);
            }

            if (context.isRunning)
            {
                ImGui::SameLine();
                if (ImGui::Button(context.paused ? "Resume" : "Pause"))
                {
                    context.paused = !context.paused;

                    if (context.paused && motion)
                        motion->setPinnedParticle(std::nullopt);

                    if (context.paused)
                    {
                        drag.active = false;
                        drag.index.reset();
                    }
                }
            }
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
                    if (motion && drag.active)
                        motion->setPinnedParticle(std::nullopt);

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
        particleShader.destroy();
        textureManager.clear();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    glm::vec3 ProjectToNDC(const glm::vec3& position)
    {
        if (cameraMode == CameraMode::ThreeD)
            return camera3D.worldToNDC(position, glm::vec2(resolution));

        return camera.worldToNDC(position);
    }

    glm::vec2 ParticleRadiusNDC(const glm::vec3& position, float radius)
    {
        glm::vec3 center = ProjectToNDC(position);

        if (cameraMode == CameraMode::ThreeD)
        {
            glm::vec3 right = ProjectToNDC(position + camera3D.getRight() * radius);
            glm::vec3 up = ProjectToNDC(position + camera3D.getUp() * radius);

            return {
                glm::length(glm::vec2(right - center)),
                glm::length(glm::vec2(up - center))
            };
        }

        glm::vec3 right = ProjectToNDC(position + glm::vec3{ radius, 0.f, 0.f });
        glm::vec3 up = ProjectToNDC(position + glm::vec3{ 0.f, radius, 0.f });

        return {
            std::abs(right.x - center.x),
            std::abs(up.y - center.y)
        };
    }

    float DepthCue(const glm::vec3& position)
    {
        if (cameraMode != CameraMode::ThreeD)
            return 1.0f;

        float depth = glm::dot(position - camera3D.getPosition(), camera3D.getForward());
        float nearDepth = std::max(Constants::Physics::DRAG_MIN_DEPTH, 1.0f);
        float farDepth = std::max(world.getDepth() + 1200.f, nearDepth + 1.0f);
        return std::clamp(1.0f - (depth - nearDepth) / farDepth, 0.28f, 1.0f);
    }

    glm::vec3 ParticleLightDirection(const glm::vec3& position)
    {
        glm::vec3 worldDirection = -camera3D.getForward() + camera3D.getUp() * 0.35f + camera3D.getRight() * -0.25f;

        if (!lights.empty())
            worldDirection = lights.front().position - position;

        if (glm::dot(worldDirection, worldDirection) <= Constants::Math::EPSILON)
            worldDirection = -camera3D.getForward();

        worldDirection = glm::normalize(worldDirection);

        if (cameraMode == CameraMode::ThreeD)
        {
            return glm::normalize(glm::vec3{
                glm::dot(worldDirection, camera3D.getRight()),
                glm::dot(worldDirection, camera3D.getUp()),
                glm::dot(worldDirection, -camera3D.getForward())
            });
        }

        return glm::normalize(glm::vec3{ worldDirection.x, worldDirection.y, 0.65f });
    }

    bool IsAntiAliasingEnabled()
    {
        return context.antiAliasing;
    }

    bool IsParticleVisible(const glm::vec3& position, float radius)
    {
        if (cameraMode == CameraMode::ThreeD)
            return camera3D.isSphereInFront(position, radius);

        glm::vec3 ndc = camera.worldToNDC(position);
        glm::vec2 ndcRadius = ParticleRadiusNDC(position, radius);

        return ndc.x + ndcRadius.x >= -1.f && ndc.x - ndcRadius.x <= 1.f &&
               ndc.y + ndcRadius.y >= -1.f && ndc.y - ndcRadius.y <= 1.f;
    }

    void AddWindowShake(glm::vec2 delta)
    {
        glm::vec3 acceleration{ -delta.x,
                                delta.y,
                                cameraMode == CameraMode::ThreeD ? -delta.x * 0.35f : 0.f };

        glm::vec3 impulse = acceleration * Constants::Physics::WINDOW_SHAKE_FORCE_SCALE;
        float impulseLength = glm::length(impulse);

        if (impulseLength > Constants::Physics::WINDOW_SHAKE_MAX_ACCELERATION)
            impulse = (impulse / impulseLength) * Constants::Physics::WINDOW_SHAKE_MAX_ACCELERATION;

        windowShakeAcceleration = windowShakeAcceleration * 0.65f + impulse * 0.35f;
    }
}
