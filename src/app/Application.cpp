#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "ui/ImGuiLayer.hpp"
#include "ui/InputManager.hpp"

#include "app/Application.hpp"

namespace Application {
    namespace {
        constexpr std::int32_t SHADOW_MAP_SIZE = 1024;

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

        void initShadowMap()
        {
            if (shadowFramebuffer != 0 && shadowDepthTexture != 0)
                return;

            glGenFramebuffers(1, &shadowFramebuffer);
            glGenTextures(1, &shadowDepthTexture);

            glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         GL_DEPTH_COMPONENT,
                         SHADOW_MAP_SIZE,
                         SHADOW_MAP_SIZE,
                         0,
                         GL_DEPTH_COMPONENT,
                         GL_FLOAT,
                         nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTexture, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDrawBuffer(GL_BACK);
            glReadBuffer(GL_BACK);
        }

        void destroyShadowMap()
        {
            if (shadowDepthTexture != 0)
            {
                glDeleteTextures(1, &shadowDepthTexture);
                shadowDepthTexture = 0;
            }

            if (shadowFramebuffer != 0)
            {
                glDeleteFramebuffers(1, &shadowFramebuffer);
                shadowFramebuffer = 0;
            }

            shadowMapReady = false;
        }

        glm::mat4 computeLightSpaceMatrix(const PointLight& light)
        {
            glm::vec3 center = world.getPosition();
            glm::vec2 halfSize = world.getHalfSize();

            float32_t halfDepth = std::max(world.getHalfDepth(), 1.0f);
            float32_t sceneRadius = glm::length(glm::vec3{ halfSize.x, halfSize.y, halfDepth });

            glm::vec3 lightDir = center - light.position;
            if (glm::dot(lightDir, lightDir) <= Constants::Math::EPSILON)
                lightDir = glm::vec3{ -0.35f, -0.75f, -0.55f };

            lightDir = glm::normalize(lightDir);

            glm::vec3 up{ 0.f, 1.f, 0.f };
            if (std::abs(glm::dot(up, lightDir)) > 0.92f)
                up = glm::vec3{ 0.f, 0.f, 1.f };

            float32_t lightDistance = std::max(glm::length(light.position - center), sceneRadius);

            glm::mat4 lightView = glm::lookAt(light.position, light.position + lightDir, up);
            glm::mat4 lightProjection = glm::ortho(-sceneRadius,
                                                   sceneRadius,
                                                   -sceneRadius,
                                                   sceneRadius,
                                                   0.1f,
                                                   lightDistance + sceneRadius * 2.0f);
            return lightProjection * lightView;
        }

        void drawParticleShadowDepth(const Particle& particle, const PointLight& light)
        {
            glm::vec3 lightDir = particle.getPosition() - light.position;
            float32_t distanceSq = glm::dot(lightDir, lightDir);

            if (distanceSq <= Constants::Math::EPSILON || distanceSq > light.range * light.range)
                return;

            lightDir = glm::normalize(lightDir);

            glm::vec3 up{ 0.f, 1.f, 0.f };
            if (std::abs(glm::dot(up, lightDir)) > 0.92f)
                up = glm::vec3{ 0.f, 0.f, 1.f };

            glm::vec3 right = glm::normalize(glm::cross(up, lightDir));
            glm::vec3 discUp = glm::normalize(glm::cross(lightDir, right));
            glm::vec3 nearestCenter = particle.getPosition() - lightDir * particle.getRadius();

            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(nearestCenter.x, nearestCenter.y, nearestCenter.z);

            constexpr std::int32_t segments = 32;
            for (std::int32_t i = 0; i <= segments; ++i)
            {
                float32_t angle = (2.0f * Constants::Math::PI * i) / static_cast<float32_t>(segments);

                glm::vec3 point = nearestCenter +
                                  right * (std::cos(angle) * particle.getRadius()) +
                                  discUp * (std::sin(angle) * particle.getRadius());

                glVertex3f(point.x, point.y, point.z);
            }

            glEnd();
        }

        void renderShadowMap()
        {
            shadowMapReady = false;

            if (!motion || lights.empty() || shadowFramebuffer == 0 || shadowDepthTexture == 0)
                return;

            const PointLight& primaryLight = lights.front();

            shadowLightSpaceMatrix = computeLightSpaceMatrix(primaryLight);

            glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
            glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
            glClear(GL_DEPTH_BUFFER_BIT);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);

            shadowShader.use();
            shadowShader.setMat4("u_lightSpaceMatrix", shadowLightSpaceMatrix);

            for (const Particle& particle : motion->particles())
                drawParticleShadowDepth(particle, primaryLight);

            shadowShader.stop();

            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDrawBuffer(GL_BACK);
            glReadBuffer(GL_BACK);
            glViewport(0, 0, resolution.x, resolution.y);
            glDisable(GL_DEPTH_TEST);

            shadowMapReady = true;
        }

        glm::vec3 mouseToFieldPoint(double mouseX, double mouseY)
        {
            if (cameraMode == CameraMode::ThreeD)
            {
                Ray ray = camera3D.screenPointToRay(mouseX, mouseY, glm::vec2(resolution));

                if (std::fabs(ray.direction.z) > Constants::Math::EPSILON)
                {
                    float32_t t = -ray.origin.z / ray.direction.z;

                    if (t >= 0.f)
                        return ray.origin + ray.direction * t;
                }

                return ray.origin + ray.direction * 300.f;
            }

            float32_t ndcX = (2.f * static_cast<float32_t>(mouseX) / resolution.x) - 1.f;
            float32_t ndcY = 1.f - (2.f * static_cast<float32_t>(mouseY) / resolution.y);

            return camera.ndcToWorld({ ndcX, ndcY, 0.f });
        }

        PointLight makePointLight(const glm::vec3& position)
        {
            return { position,
                     Constants::Lighting::DEFAULT_RADIUS,
                     Constants::Lighting::DEFAULT_RANGE,
                     Constants::Lighting::DEFAULT_INTENSITY,
                     nextLightId++ };
        }

        std::optional<std::size_t> pickLight2D(const glm::vec3& worldPoint)
        {
            float32_t bestDistSq = std::numeric_limits<float32_t>::max();
            std::optional<std::size_t> bestIndex;

            for (std::size_t i = 0; i < lights.size(); ++i)
            {
                glm::vec3 delta = lights[i].position - worldPoint;
                float32_t radius = lights[i].radius * 1.8f;
                float32_t distSq = glm::dot(delta, delta);

                if (distSq <= radius * radius && distSq < bestDistSq)
                {
                    bestDistSq = distSq;
                    bestIndex = i;
                }
            }

            return bestIndex;
        }

        std::optional<std::size_t> pickLight3D(const Ray& ray)
        {
            float32_t closestT = std::numeric_limits<float32_t>::max();
            std::optional<std::size_t> bestIndex;

            for (std::size_t i = 0; i < lights.size(); ++i)
            {
                const PointLight& light = lights[i];

                glm::vec3 oc = ray.origin - light.position;

                float32_t pickRadius = light.radius * 2.1f;
                float32_t b = glm::dot(oc, ray.direction);
                float32_t c = glm::dot(oc, oc) - pickRadius * pickRadius;
                float32_t h = b * b - c;

                if (h < 0.0f)
                    continue;

                float32_t sqrtH = std::sqrt(h);
                float32_t t = -b - sqrtH;

                if (t < 0.0f)
                    t = -b + sqrtH;

                if (t >= 0.0f && t < closestT)
                {
                    closestT = t;
                    bestIndex = i;
                }
            }

            return bestIndex;
        }

        std::optional<std::size_t> pickLightAt(double mouseX, double mouseY, const glm::vec3& fieldPoint)
        {
            if (cameraMode == CameraMode::ThreeD)
                return pickLight3D(camera3D.screenPointToRay(mouseX, mouseY, glm::vec2(resolution)));

            return pickLight2D(fieldPoint);
        }

        void drawCircleNdc(const glm::vec3& center, const glm::vec2& radius, float32_t r, float32_t g, float32_t b, float32_t a)
        {
            if (!std::isfinite(center.x) || !std::isfinite(center.y) ||
                !std::isfinite(radius.x) || !std::isfinite(radius.y) ||
                radius.x <= 0.f || radius.y <= 0.f) return;

            glColor4f(r, g, b, a);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(center.x, center.y);

            constexpr std::int32_t segments = 36;
            for (std::int32_t i = 0; i <= segments; ++i)
            {
                float32_t angle = (2.f * Constants::Math::PI * i) / segments;

                glVertex2f(center.x + std::cos(angle) * radius.x,
                           center.y + std::sin(angle) * radius.y);
            }

            glEnd();
        }

        float32_t lineDepthFade(const glm::vec3& a, const glm::vec3& b)
        {
            if (cameraMode != CameraMode::ThreeD)
                return 1.0f;

            float32_t depthA = glm::dot(a - camera3D.getPosition(), camera3D.getForward());
            float32_t depthB = glm::dot(b - camera3D.getPosition(), camera3D.getForward());
            float32_t depth = (depthA + depthB) * 0.5f;
            float32_t farDepth = std::max(world.getDepth() + 900.f, 1.0f);

            return std::clamp(1.0f - depth / farDepth, 0.16f, 0.78f);
        }

        void drawProjectedLine(const glm::vec3& a,
                               const glm::vec3& b,
                               float32_t r,
                               float32_t g,
                               float32_t bl,
                               float32_t alpha,
                               bool distort = false)
        {
            if (cameraMode == CameraMode::ThreeD &&
                !camera3D.isSphereInFront((a + b) * 0.5f, glm::length(b - a) * 0.5f))
            {
                return;
            }

            constexpr std::int32_t segments = 18;

            glColor4f(r, g, bl, alpha * lineDepthFade(a, b));
            glBegin(GL_LINE_STRIP);

            for (std::int32_t i = 0; i <= segments; ++i)
            {
                float32_t t = static_cast<float32_t>(i) / static_cast<float32_t>(segments);
                glm::vec3 p = a + (b - a) * t;

                if (distort && cameraMode == CameraMode::ThreeD)
                {
                    float32_t depth = glm::dot(p - camera3D.getPosition(), camera3D.getForward());
                    float32_t distanceScale = std::clamp(depth / std::max(world.getDepth(), 1.0f), 0.0f, 1.0f);
                    float32_t wave = std::sin(t * Constants::Math::PI * 6.0f + depth * 0.018f);
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

            constexpr float32_t nearDepth = 2.0f;

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

                float32_t currentDepth = depthOf(current);
                float32_t nextDepth = depthOf(next);
                bool currentInside = currentDepth > nearDepth;
                bool nextInside = nextDepth > nearDepth;

                if (currentInside && nextInside)
                {
                    clipped.push_back(next);
                }
                else if (currentInside != nextInside)
                {
                    float32_t t = (nearDepth - currentDepth) / (nextDepth - currentDepth);
                    clipped.push_back(current + (next - current) * t);

                    if (nextInside)
                        clipped.push_back(next);
                }
            }

            return clipped;
        }

        void drawProjectedFace(const std::vector<glm::vec3>& face, float32_t r, float32_t g, float32_t b, float32_t alpha)
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

            float32_t halfDepth = world.getHalfDepth();

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

            const float32_t x0 = min.x, x1 = max.x;
            const float32_t y0 = min.y, y1 = max.y;
            const float32_t z0 = min.z, z1 = max.z;

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

        void drawFreeFallFloorShadows()
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

            float32_t halfDepth = world.getHalfDepth();

            if (halfDepth <= Constants::Math::EPSILON)
                return;

            const float32_t floorY = center.y - halfSize.y;
            const float32_t minX = center.x - halfSize.x;
            const float32_t maxX = center.x + halfSize.x;
            const float32_t minZ = center.z - halfDepth;
            const float32_t maxZ = center.z + halfDepth;

            particleShader.stop();

            glDisable(GL_TEXTURE_2D);

            for (const PointLight& light : lights)
            {
                if (light.position.y <= floorY + Constants::Math::EPSILON)
                    continue;

                for (const Particle& particle : motion->particles())
                {
                    glm::vec3 lightToParticle = particle.getPosition() - light.position;
                    float32_t particleDistance = glm::length(lightToParticle);

                    if (particleDistance <= Constants::Math::EPSILON || particleDistance > light.range)
                        continue;

                    if (lightToParticle.y >= -Constants::Math::EPSILON)
                        continue;

                    float32_t t = (floorY - light.position.y) / lightToParticle.y;

                    if (t <= 1.0f)
                        continue;

                    glm::vec3 hit = light.position + lightToParticle * t;

                    if (hit.x < minX || hit.x > maxX || hit.z < minZ || hit.z > maxZ)
                        continue;

                    float32_t rangeFade = std::clamp(1.0f - particleDistance / std::max(light.range, 1.0f), 0.0f, 1.0f);
                    float32_t projectionScale = std::clamp(t, 1.0f, 5.5f);
                    float32_t shadowRadius = std::clamp(particle.getRadius() * projectionScale, 3.0f, 180.0f);
                    float32_t alpha = std::clamp(light.intensity * rangeFade * rangeFade * 0.30f / std::sqrt(projectionScale),
                                             0.0f,
                                             0.34f);

                    if (alpha <= 0.002f)
                        continue;

                    std::vector<glm::vec3> shadow;
                    shadow.reserve(28);

                    constexpr std::int32_t segments = 28;
                    for (std::int32_t i = 0; i < segments; ++i)
                    {
                        float32_t angle = (2.0f * Constants::Math::PI * i) / static_cast<float32_t>(segments);
                        float32_t x = std::clamp(hit.x + std::cos(angle) * shadowRadius, minX, maxX);
                        float32_t z = std::clamp(hit.z + std::sin(angle) * shadowRadius, minZ, maxZ);
                        shadow.push_back({ x, floorY + 0.05f, z });
                    }

                    drawProjectedFace(shadow, 0.0f, 0.0f, 0.0f, alpha);
                }
            }

            glColor4f(1.f, 1.f, 1.f, 1.f);
        }

        void drawLighting()
        {
            if (!motion || lights.empty())
                return;

            particleShader.stop();
            glDisable(GL_TEXTURE_2D);

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
        std::string shadowVertexShader = readProjectTextFile("shaders/shadow_depth.vert");
        std::string shadowFragmentShader = readProjectTextFile("shaders/shadow_depth.frag");

        if (vertexShader.empty() || fragmentShader.empty() ||
            !particleShader.init(vertexShader, fragmentShader))
        {
            std::fprintf(stderr, "Failed to initialize particle shader.\n");
            return false;
        }

        if (shadowVertexShader.empty() || shadowFragmentShader.empty() ||
            !shadowShader.init(shadowVertexShader, shadowFragmentShader))
        {
            std::fprintf(stderr, "Failed to initialize shadow shader.\n");
            return false;
        }

        initShadowMap();

        globalParticleTexture = textureManager.createSolid("White", 255, 255, 255, 255);

        return true;
    }

    void Update(float32_t dt)
    {
        float32_t frameDt = std::min(dt, Constants::Physics::MAX_FRAME_TIME);
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
                camera3D.processMouseDelta(static_cast<float32_t>(dx), static_cast<float32_t>(dy));
        } else
        {
            double dx = 0.0, dy = 0.0;
            input.consumeMouseDelta(dx, dy);
        }

        if (input.wasMousePressed(GLFW_MOUSE_BUTTON_LEFT) && !ImGui::GetIO().WantCaptureMouse)
        {
            double mouseX = 0.0, mouseY = 0.0;
            input.cursorPos(mouseX, mouseY);
            glm::vec3 fieldPoint = mouseToFieldPoint(mouseX, mouseY);

            lightDrag.index = pickLightAt(mouseX, mouseY, fieldPoint);
            lightDrag.active = lightDrag.index.has_value();

            if (lightDrag.active)
            {
                if (drag.active && motion)
                    motion->setPinnedParticle(std::nullopt);

                drag.active = false;
                drag.index.reset();

                selectedLight = lightDrag.index;
                showLightWindow = true;
                selectedParticle.reset();

                showParticleWindow = false;
            }

            if (lightDrag.active && cameraMode == CameraMode::ThreeD && lightDrag.index.has_value())
            {
                const PointLight& light = lights[*lightDrag.index];
                lightDrag.depth = std::max(glm::dot(light.position - camera3D.getPosition(),
                                           camera3D.getForward()),
                                           Constants::Physics::DRAG_MIN_DEPTH);
            }

            if (!lightDrag.active && motion && cameraMode == CameraMode::ThreeD)
            {
                Ray ray = camera3D.screenPointToRay(mouseX, mouseY, glm::vec2(resolution));
                selectedParticle = motion->pickParticle(ray.origin, ray.direction);
            } else if (!lightDrag.active && motion)
            {
                selectedParticle = motion->pickParticle2D(fieldPoint);
            }

            if (motion && !context.paused && !lightDrag.active && selectedParticle.has_value())
            {
                showParticleWindow = true;
                selectedLight.reset();
                showLightWindow = false;

                if (cameraMode == CameraMode::ThreeD)
                {
                    Particle* particle = motion->getParticleMutable(*selectedParticle);

                    if (particle)
                    {
                        lightDrag.active = false;
                        lightDrag.index.reset();

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
                        float32_t maxDepth = world.getDepth() + glm::length(glm::vec2(world.getSize())) + 1200.f;
                        lightDrag.depth = std::clamp(lightDrag.depth -
                                                         static_cast<float32_t>(scrollDelta) * Constants::Physics::DRAG_SCROLL_DEPTH_STEP,
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
            else
            {
                lightDrag.active = false;
                lightDrag.index.reset();
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
                    float32_t maxDepth = world.getDepth() + glm::length(glm::vec2(world.getSize())) + 1200.f;
                    drag.depth = std::clamp(drag.depth -
                                                static_cast<float32_t>(scrollDelta) * Constants::Physics::DRAG_SCROLL_DEPTH_STEP,
                                            Constants::Physics::DRAG_MIN_DEPTH,
                                            maxDepth);
                }

                Ray ray = camera3D.screenPointToRay(mouseX, mouseY, glm::vec2(resolution));
                glm::vec3 newPos = ray.origin + ray.direction * drag.depth;
                Particle* particle = motion->getParticleMutable(*drag.index);

                if (particle)
                {
                    drag.targetPosition = newPos;

                    float32_t dragDt = std::max(frameDt, Constants::Physics::MIN_DT);
                    float32_t follow = 1.f - std::exp(-Constants::Physics::DRAG_FOLLOW_STIFFNESS * dragDt);
                    glm::vec3 smoothedPos = particle->getPosition() + (drag.targetPosition - particle->getPosition()) * follow;
                    glm::vec3 velocity = (smoothedPos - drag.lastPosition) / dragDt;

                    float32_t speed = glm::length(velocity);
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

            static float32_t accumulator = 0.0f;
            accumulator += frameDt;

            const float32_t step = Constants::Physics::FIXED_TIME_STEP;

            while (accumulator >= step)
            {
                motion->update(step);
                accumulator -= step;
            }
        }
    }

    void Render() {
        renderShadowMap();

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (context.antiAliasing)
            glEnable(GL_MULTISAMPLE);
        else
            glDisable(GL_MULTISAMPLE);

        drawFreeFallBounds();
        drawFreeFallFloorShadows();
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
            std::int32_t mode = (cameraMode == CameraMode::TwoD) ? 0 : 1;

            ImGui::RadioButton("Camera 2D", &mode, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Camera 3D", &mode, 1);

            cameraMode = (mode == 0) ? CameraMode::TwoD : CameraMode::ThreeD;
            updateFieldDepthForCameraMode();

            bool lockSimulationConfig = context.isRunning && !context.paused;
            std::int32_t simMode = (context.pendingSimulationMode == Simulation::Mode::FreeFall) ? 0 : 1;

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
                lights.push_back(makePointLight(world.getPosition()));
            }

            ImGui::SameLine();
            if (ImGui::Button("Clear Lights"))
            {
                lights.clear();
                lightDrag.active = false;
                lightDrag.index.reset();
                selectedLight.reset();
                showLightWindow = false;
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
                selectedLight.reset();

                showParticleWindow = false;
                showLightWindow = false;

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

        if (showLightWindow && selectedLight.has_value())
        {
            if (*selectedLight >= lights.size())
            {
                showLightWindow = false;
                selectedLight.reset();
            }
            else
            {
                PointLight& light = lights[*selectedLight];

                if (ImGui::Begin("Light", &showLightWindow))
                {
                    ImGui::Text("Id: %u", light.id);
                    ImGui::Text("Position: (%.2f, %.2f, %.2f)", light.position.x, light.position.y, light.position.z);
                    ImGui::SliderFloat("Intensity", &light.intensity, 0.0f, 2.0f, "%.2f");
                    ImGui::SliderFloat("Range",
                                       &light.range,
                                       Constants::Lighting::MIN_RANGE,
                                       Constants::Lighting::MAX_RANGE,
                                       "%.0f");
                }

                ImGui::End();

                if (!showLightWindow)
                    selectedLight.reset();
            }
        }

        ImGuiLayer::EndFrame();

        glfwSwapBuffers(window);
    }

    void Tick(float32_t dt)
    {
        Update(dt);
        Render();
    }

    void Cleanup()
    {
        ImGuiLayer::Shutdown();

        particleShader.destroy();
        shadowShader.destroy();

        destroyShadowMap();

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

    glm::vec2 ParticleRadiusNDC(const glm::vec3& position, float32_t radius)
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

    float32_t DepthCue(const glm::vec3& position)
    {
        if (cameraMode != CameraMode::ThreeD)
            return 1.0f;

        float32_t depth = glm::dot(position - camera3D.getPosition(), camera3D.getForward());
        float32_t nearDepth = std::max(Constants::Physics::DRAG_MIN_DEPTH, 1.0f);
        float32_t farDepth = std::max(world.getDepth() + 1200.f, nearDepth + 1.0f);

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

    bool IsParticleVisible(const glm::vec3& position, float32_t radius)
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
        float32_t impulseLength = glm::length(impulse);

        if (impulseLength > Constants::Physics::WINDOW_SHAKE_MAX_ACCELERATION)
            impulse = (impulse / impulseLength) * Constants::Physics::WINDOW_SHAKE_MAX_ACCELERATION;

        windowShakeAcceleration = windowShakeAcceleration * 0.65f + impulse * 0.35f;
    }
}
