#include <algorithm>
#include <cmath>
#include <GLFW/glfw3.h>

#include "app/Application.hpp"
#include "app/Constants.hpp"

#include "physics/Particle.hpp"

Particle::Particle(const glm::vec3& position, const glm::vec3& velocity, float mass, float radius, float angularVelocity)
    : m_id(m_nextId++),
      m_force(0.f),
      m_position(position), 
      m_velocity(velocity),
      m_mass(std::max(mass, Constants::Physics::MIN_MASS)),
      m_radius(radius),
      m_rotation(0.f),
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
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    Application::particleShader.setInt("u_texture", 0);
    Application::particleShader.setVec4("u_tint", 1.f, 1.f, 1.f, 1.f);

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

        float textureAngle = angle + m_rotation;
        float u = 0.5f + std::cos(textureAngle) * 0.5f;
        float v = 0.5f + std::sin(textureAngle) * 0.5f;

        glTexCoord2f(u, v);
        glVertex2f(ndc.x + dx, ndc.y + dy);
    }

    glEnd();

    Application::particleShader.stop();

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
    m_rotation = std::fmod(m_rotation + m_angularVelocity * dt, 2.0f * Constants::Math::PI);

    if (m_rotation < 0.0f)
        m_rotation += 2.0f * Constants::Math::PI;
}

void Particle::setMass(float m) noexcept
{
    m_mass = std::max(m, Constants::Physics::MIN_MASS);
}
