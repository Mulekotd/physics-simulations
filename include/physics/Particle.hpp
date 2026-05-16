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
                      float radius = 1.0f);

    [[nodiscard]] Id getId() const noexcept { return m_id; }
    [[nodiscard]] const glm::vec3& getPosition() const noexcept { return m_position; }
    [[nodiscard]] const glm::vec3& getVelocity() const noexcept { return m_velocity; }

    [[nodiscard]] float getMass() const noexcept { return m_mass; }
    [[nodiscard]] float getRadius() const noexcept { return m_radius; }
    [[nodiscard]] std::optional<TextureId> getTexture() const noexcept { return m_texture; }

    void setPosition(const glm::vec3& p) noexcept { m_position = p; }
    void setVelocity(const glm::vec3& v) noexcept { m_velocity = v; }
    void setMass(float m) noexcept;
    void setRadius(float r) noexcept { m_radius = r; }
    void setTexture(TextureId id) noexcept { m_texture = id; }
    void clearTexture() noexcept { m_texture.reset(); }

    void draw() const;
    
    void addForce(const glm::vec3& f) noexcept { m_force += f; }
    void clearForces() noexcept { m_force = {}; }
    void integrate(float dt) noexcept;

private:
    Id        m_id;
    glm::vec3 m_force, m_position, m_velocity;
    float     m_mass, m_radius;
    std::optional<TextureId> m_texture;

    inline static Id m_nextId{0};
};
