#include <algorithm>
#include <cmath>
#include <GLFW/glfw3.h>

#include "app/Application.hpp"
#include "app/Constants.hpp"

#include "physics/Particle.hpp"

Particle::Particle(const glm::vec3& position, const glm::vec3& velocity, float mass, float radius)
    : m_id(m_nextId++),
      m_force(0.f),
      m_position(position), 
      m_velocity(velocity),
      m_mass(std::max(mass, Constants::Physics::MIN_MASS)),
      m_radius(radius)
{}

void Particle::draw() const
{
    if (!Application::IsParticleVisible(m_position, m_radius))
        return;

    glm::vec3 ndc = Application::ProjectToNDC(m_position);
    glm::vec3 ndcEdge = Application::ProjectToNDC(m_position + Application::ParticleRadiusAxis() * m_radius);

    float ndcRadius = glm::length(glm::vec2(ndcEdge - ndc));

    if (!std::isfinite(ndc.x) || !std::isfinite(ndc.y) || !std::isfinite(ndcRadius) || ndcRadius <= 0.f)
        return;

    Application::particleShader.use();

    TextureId texture = m_texture.value_or(Application::globalParticleTexture);
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
        float dx = std::cos(angle) * ndcRadius;
        float dy = std::sin(angle) * ndcRadius;

        float u = 0.5f + std::cos(angle) * 0.5f;
        float v = 0.5f + std::sin(angle) * 0.5f;

        glTexCoord2f(u, v);
        glVertex2f(ndc.x + dx, ndc.y + dy);
    }

    glEnd();
}

void Particle::integrate(float dt) noexcept
{
    // resulting acceleration (F = m·a → a = F/m)
    const glm::vec3 a = m_force * (1.0f / m_mass);

    // position with 2nd-order term (s = s₀ + v₀·dt + ½·a·dt²)
    m_position += m_velocity * dt + a * (0.5f * dt * dt);

    // exact velocity (v = v₀ + a·dt)
    m_velocity += a * dt;

    clearForces();
}

void Particle::setMass(float m) noexcept
{
    m_mass = std::max(m, Constants::Physics::MIN_MASS);
}
