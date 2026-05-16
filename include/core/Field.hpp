#pragma once

#include "common/Constants.hpp"
#include "common/Dimensions.hpp"
#include "common/Vector3.hpp"

class Particle;

class Field {
public:
    explicit Field(Dimensions size, 
                   float gravity = Constants::Physics::GRAVITY,
                   float friction = Constants::Physics::DYNAMIC_FRICTION_COEFFICIENT, 
                   float restitution = Constants::Physics::RESTITUTION_COEFFICIENT);

    ~Field() = default;

    [[nodiscard]] Dimensions getSize() const noexcept { return m_size; }
    [[nodiscard]] Vector3 getPosition() const noexcept { return m_position; }
    [[nodiscard]] float getGravityConstant() const noexcept { return m_gravity; }
    [[nodiscard]] float getFrictionConstant() const noexcept { return m_friction; }
    [[nodiscard]] float getRestitutionConstant() const noexcept { return m_restitution; }

    void setSize(Dimensions s) noexcept { m_size = s; }
    void setPosition(Vector3 p) noexcept { m_position = p; }
    void setGravityConstant(float g) noexcept { m_gravity = g; }
    void setFrictionConstant(float f) noexcept { m_friction = f; }
    void setRestitutionConstant(float r) noexcept { m_restitution = r; }

    bool contains(const Vector3& position) const noexcept;
    bool contains(const Particle& particle) const noexcept;
    Vector3 getRelativePosition(const Vector3& position) const noexcept;

    void reset();

private:
    Dimensions m_size;
    Vector3    m_position;
    float      m_gravity, m_friction, m_restitution;
};
