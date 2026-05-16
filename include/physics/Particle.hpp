#pragma once

#include <cstdint>
#include <optional>

#include <glm/vec3.hpp>

#include "engine/TextureManager.hpp"

using Id = std::uint32_t;

class Camera2D;

class Particle {
public:
    explicit Particle(const glm::vec3& position = {0, 0, 0},
                      const glm::vec3& velocity = {0, 0, 0},
                      float mass = 1.0f,
                      float radius = 1.0f,
                      float angularVelocity = 0.0f);

    [[nodiscard]] Id getId() const noexcept { return m_id; }
    [[nodiscard]] const glm::vec3& getPosition() const noexcept { return m_position; }
    [[nodiscard]] const glm::vec3& getVelocity() const noexcept { return m_velocity; }

    [[nodiscard]] float getMass() const noexcept { return m_mass; }
    [[nodiscard]] float getRadius() const noexcept { return m_radius; }
    [[nodiscard]] float getRotation() const noexcept { return m_rotation; }
    [[nodiscard]] float getAngularVelocity() const noexcept { return m_angularVelocity; }
    [[nodiscard]] std::optional<TextureId> getTexture() const noexcept { return m_texture; }

    void setPosition(const glm::vec3& p) noexcept { m_position = p; }
    void setVelocity(const glm::vec3& v) noexcept { m_velocity = v; }
    void setRotation(float rotation) noexcept { m_rotation = rotation; }
    void setAngularVelocity(float angularVelocity) noexcept { m_angularVelocity = angularVelocity; }
    void setMass(float m) noexcept;
    void setRadius(float r) noexcept { m_radius = r; }
    void setTexture(TextureId id) noexcept { m_texture = id; }
    void clearTexture() noexcept { m_texture.reset(); }

    void draw() const;
    
    void addForce(const glm::vec3& f) noexcept { m_force += f; }
    void clearForces() noexcept { m_force = {}; }
    void integrate(float dt) noexcept;
    void integrateSymplectic(float dt) noexcept;

private:
    void integrateRotation(float dt) noexcept;

    Id        m_id;
    glm::vec3 m_force, m_position, m_velocity;
    float     m_mass, m_radius, m_rotation, m_angularVelocity;
    std::optional<TextureId> m_texture;

    inline static Id m_nextId{0};
};
