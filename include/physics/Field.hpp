#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "app/Constants.hpp"

class Particle;

class Field {
public:
    struct Properties {
        float gravity;
        float friction;
        float restitution;
    };

    explicit Field(glm::vec2 size,
                   Properties props = { Constants::Physics::GRAVITY,
                                        Constants::Physics::DYNAMIC_FRICTION_COEFFICIENT,
                                        Constants::Physics::RESTITUTION_COEFFICIENT });

    ~Field() = default;

    [[nodiscard]] const glm::vec2& getSize() const noexcept { return m_size; }
    [[nodiscard]] const glm::vec2& getHalfSize() const noexcept { return m_halfSize; }
    [[nodiscard]] float getDepth() const noexcept { return m_depth; }
    [[nodiscard]] float getHalfDepth() const noexcept { return m_halfDepth; }
    [[nodiscard]] glm::vec3 getPosition() const noexcept { return m_position; }
    [[nodiscard]] float getGravityConstant() const noexcept { return m_props.gravity; }
    [[nodiscard]] float getFrictionConstant() const noexcept { return m_props.friction; }
    [[nodiscard]] float getRestitutionConstant() const noexcept { return m_props.restitution; }
    [[nodiscard]] Properties getProperties() const noexcept { return m_props; }

    void setSize(glm::vec2 s) noexcept;
    void setDepth(float depth) noexcept;
    void setPosition(glm::vec3 p) noexcept { m_position = p; }
    void setGravityConstant(float g) noexcept { m_props.gravity = g; }
    void setFrictionConstant(float f) noexcept { m_props.friction = f; }
    void setRestitutionConstant(float r) noexcept { m_props.restitution = r; }
    void setProperties(Properties props) noexcept { m_props = props; }

    bool contains(const glm::vec3& position) const noexcept;
    bool contains(const Particle& particle) const noexcept;

    glm::vec3 getRelativePosition(const glm::vec3& position) const noexcept;

    void reset();

private:
    glm::vec2  m_size;
    glm::vec2  m_halfSize;
    float      m_depth;
    float      m_halfDepth;
    glm::vec3  m_position;
    Properties m_props;
};
