#pragma once

#include <cstdint>
#include <optional>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include "app/Types.hpp"
#include "engine/TextureManager.hpp"

using Id = std::uint32_t;

class Camera2D;

class Particle {
public:
    explicit Particle(const glm::vec3& position = {0, 0, 0},
                      const glm::vec3& velocity = {0, 0, 0},
                      float32_t mass = 1.0f,
                      float32_t radius = 1.0f,
                      float32_t angularVelocity = 0.0f);

    [[nodiscard]] Id getId() const noexcept { return m_id; }
    [[nodiscard]] const glm::vec3& getPosition() const noexcept { return m_position; }
    [[nodiscard]] const glm::vec3& getVelocity() const noexcept { return m_velocity; }

    [[nodiscard]] float32_t getMass() const noexcept { return m_mass; }
    [[nodiscard]] float32_t getRadius() const noexcept { return m_radius; }
    [[nodiscard]] float32_t getRotation() const noexcept { return rotationAngle(); }
    [[nodiscard]] float32_t getAngularVelocity() const noexcept { return m_angularVelocity; }
    [[nodiscard]] std::optional<TextureId> getTexture() const noexcept { return m_texture; }

    void setPosition(const glm::vec3& p) noexcept { m_position = p; }
    void setVelocity(const glm::vec3& v) noexcept { m_velocity = v; }
    void setRotation(float32_t rotation) noexcept { m_rotation = glm::angleAxis(rotation, glm::vec3{ 0.f, 0.f, 1.f }); }
    void setAngularVelocity(float32_t angularVelocity) noexcept { m_angularVelocity = angularVelocity; }
    void setMass(float32_t m) noexcept;
    void setRadius(float32_t r) noexcept { m_radius = r; }
    void setTexture(TextureId id) noexcept { m_texture = id; }
    void clearTexture() noexcept { m_texture.reset(); }

    void draw() const;
    
    void addForce(const glm::vec3& f) noexcept { m_force += f; }
    void clearForces() noexcept { m_force = {}; }
    void integrate(float32_t dt) noexcept;
    void integrateSymplectic(float32_t dt) noexcept;

private:
    void integrateRotation(float32_t dt) noexcept;
    [[nodiscard]] float32_t rotationAngle() const noexcept;

    Id        m_id;
    glm::vec3 m_force, m_position, m_velocity;
    glm::quat m_rotation;
    float32_t     m_mass, m_radius, m_angularVelocity;
    std::optional<TextureId> m_texture;

    inline static Id m_nextId{0};
};
