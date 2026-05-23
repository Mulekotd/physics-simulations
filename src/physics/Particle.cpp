#include <algorithm>
#include <cmath>
#include <string>
#include <GLFW/glfw3.h>

#include <glm/gtc/quaternion.hpp>

#include "app/Application.hpp"
#include "app/Constants.hpp"

#include "physics/Particle.hpp"

Particle::Particle(const glm::vec3& position, const glm::vec3& velocity, float mass, float radius, float angularVelocity)
    : m_id(m_nextId++),
      m_force(0.f),
      m_position(position), 
      m_velocity(velocity),
      m_rotation(1.f, 0.f, 0.f, 0.f),
      m_mass(std::max(mass, Constants::Physics::MIN_MASS)),
      m_radius(radius),
      m_angularVelocity(angularVelocity)
{}

void Particle::draw() const
{
    if (!Application::IsParticleVisible(m_position, m_radius))
        return;

    glm::vec3 ndc = Application::ProjectToNDC(m_position);
    glm::vec2 ndcRadius = Application::ParticleRadiusNDC(m_position, m_radius);

    if (!std::isfinite(ndc.x) || !std::isfinite(ndc.y) ||
        !std::isfinite(ndcRadius.x) || !std::isfinite(ndcRadius.y) ||
        ndcRadius.x <= 0.f || ndcRadius.y <= 0.f)
    {
        return;
    }

    Application::particleShader.use();

    TextureId texture = m_texture.value_or(Application::globalParticleTexture);
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    Application::particleShader.setInt("u_texture", 0);
    Application::particleShader.setInt("u_shadowMap", 1);
    Application::particleShader.setVec4("u_tint", 1.f, 1.f, 1.f, 1.f);
    Application::particleShader.setFloat("u_depthCue", Application::DepthCue(m_position));
    Application::particleShader.setInt("u_antialias", Application::IsAntiAliasingEnabled() ? 1 : 0);
    Application::particleShader.setVec3("u_particleCenter", m_position.x, m_position.y, m_position.z);
    Application::particleShader.setFloat("u_particleRadius", m_radius);

    glm::vec3 viewRight{ 1.f, 0.f, 0.f };
    glm::vec3 viewUp{ 0.f, 1.f, 0.f };
    glm::vec3 viewForward{ 0.f, 0.f, -1.f };
    glm::vec3 viewPosition{ m_position.x, m_position.y, m_position.z + 1.f };

    if (Application::cameraMode == Application::CameraMode::ThreeD)
    {
        viewRight = Application::camera3D.getRight();
        viewUp = Application::camera3D.getUp();
        viewForward = Application::camera3D.getForward();
        viewPosition = Application::camera3D.getPosition();
    }

    Application::particleShader.setVec3("u_viewRight", viewRight.x, viewRight.y, viewRight.z);
    Application::particleShader.setVec3("u_viewUp", viewUp.x, viewUp.y, viewUp.z);
    Application::particleShader.setVec3("u_viewForward", viewForward.x, viewForward.y, viewForward.z);
    Application::particleShader.setVec3("u_viewPosition", viewPosition.x, viewPosition.y, viewPosition.z);
    Application::particleShader.setMat4("u_lightSpaceMatrix", Application::shadowLightSpaceMatrix);
    Application::particleShader.setFloat("u_shadowMapSize", 1024.0f);
    Application::particleShader.setInt("u_shadowEnabled", Application::shadowMapReady ? 1 : 0);

    int lightCount = std::min(static_cast<int>(Application::lights.size()), Constants::Lighting::MAX_LIGHTS);
    Application::particleShader.setInt("u_lightCount", lightCount);

    for (int i = 0; i < lightCount; ++i)
    {
        const auto& light = Application::lights[static_cast<std::size_t>(i)];
        std::string index = std::to_string(i);

        Application::particleShader.setVec3("u_lightPositions[" + index + "]",
                                            light.position.x,
                                            light.position.y,
                                            light.position.z);
        Application::particleShader.setFloat("u_lightRanges[" + index + "]", light.range);
        Application::particleShader.setFloat("u_lightIntensities[" + index + "]", light.intensity);
    }

    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, Application::shadowDepthTexture);
    glActiveTexture(GL_TEXTURE0);

    glBegin(GL_TRIANGLE_FAN);

    glTexCoord2f(0.5f, 0.5f);
    glVertex2f(ndc.x, ndc.y);

    // Circumference
    int segments = std::clamp(static_cast<int>(m_radius * 10), 24, 128);
    float step = 2.0f * Constants::Math::PI / segments;

    for (int i = 0; i <= segments; ++i)
    {
        float angle = i * step;
        float dx = std::cos(angle) * ndcRadius.x;
        float dy = std::sin(angle) * ndcRadius.y;

        float textureAngle = angle + rotationAngle();
        float u = 0.5f + std::cos(textureAngle) * 0.5f;
        float v = 0.5f + std::sin(textureAngle) * 0.5f;

        glTexCoord2f(u, v);
        glVertex2f(ndc.x + dx, ndc.y + dy);
    }

    glEnd();

    Application::particleShader.stop();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
}

void Particle::integrate(float dt) noexcept
{
    // resulting acceleration (F = m·a → a = F/m)
    const glm::vec3 a = m_force * (1.0f / m_mass);

    // position with 2nd-order term (s = s₀ + v₀·dt + ½·a·dt²)
    m_position += m_velocity * dt + a * (0.5f * dt * dt);

    // exact velocity (v = v₀ + a·dt)
    m_velocity += a * dt;

    integrateRotation(dt);

    clearForces();
}

void Particle::integrateSymplectic(float dt) noexcept
{
    const glm::vec3 a = m_force * (1.0f / m_mass);

    m_velocity += a * dt;
    m_position += m_velocity * dt;

    integrateRotation(dt);
    clearForces();
}

void Particle::integrateRotation(float dt) noexcept
{
    if (std::abs(m_angularVelocity) <= Constants::Math::EPSILON)
        return;

    glm::quat delta = glm::angleAxis(m_angularVelocity * dt, glm::vec3{ 0.f, 0.f, 1.f });
    m_rotation = glm::normalize(delta * m_rotation);
}

float Particle::rotationAngle() const noexcept
{
    glm::vec3 rotatedX = m_rotation * glm::vec3{ 1.f, 0.f, 0.f };
    float angle = std::atan2(rotatedX.y, rotatedX.x);

    if (angle < 0.0f)
        angle += 2.0f * Constants::Math::PI;

    return angle;
}

void Particle::setMass(float m) noexcept
{
    m_mass = std::max(m, Constants::Physics::MIN_MASS);
}
