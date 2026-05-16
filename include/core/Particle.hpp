#pragma once

#include <cstdint>

#include "common/Types.hpp"
#include "common/Vector3.hpp"

class Camera2D;

class Particle {
public:
    explicit Particle(const Vector3& position = {0, 0, 0},
                      const Vector3& velocity = {0, 0, 0},
                      float mass = 1.0f,
                      float radius = 1.0f);

    [[nodiscard]] Id getId() const noexcept { return m_id; }
    [[nodiscard]] const Vector3& getPosition() const noexcept { return m_position; }
    [[nodiscard]] const Vector3& getVelocity() const noexcept { return m_velocity; }

    [[nodiscard]] float getMass() const noexcept { return m_mass; }
    [[nodiscard]] float getRadius() const noexcept { return m_radius; }

    void setPosition(const Vector3& p) noexcept { m_position = p; }
    void setVelocity(const Vector3& v) noexcept { m_velocity = v; }
    void setMass(float m) noexcept { m_mass = m; }
    void setRadius(float r) noexcept { m_radius = r; }

    void draw() const;
    
    void addForce(const Vector3& f) noexcept { m_force += f; }
    void clearForces() noexcept { m_force = {}; }
    void integrate(float dt) noexcept;

private:
    Id       m_id; // TODO: Change id to uuidv4
    Vector3  m_force, m_position, m_velocity;
    float    m_mass, m_radius;

    inline static Id m_nextId{0};
};
