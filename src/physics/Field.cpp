#include <algorithm>

#include "physics/Field.hpp"
#include "physics/Particle.hpp"

Field::Field(glm::vec2 size, Properties props)
        : m_size(size),
          m_halfSize(size * 0.5f),
          m_depth(Constants::Simulation::FIELD_DEPTH_2D),
          m_halfDepth(m_depth * 0.5f),
          m_position(glm::vec3(m_halfSize, 0.f)),
          m_props(props)
{}

void Field::setSize(glm::vec2 s) noexcept
{
        m_size = s;
        m_halfSize = s * 0.5f;
}

void Field::setDepth(float32_t depth) noexcept
{
    m_depth = std::max(depth, 0.0f);
    m_halfDepth = m_depth * 0.5f;
}

void Field::reset()
{
    m_props = { Constants::Physics::GRAVITY,
                Constants::Physics::DYNAMIC_FRICTION_COEFFICIENT,
                Constants::Physics::RESTITUTION_COEFFICIENT };
    m_position = glm::vec3(m_halfSize, 0.f);
}

bool Field::contains(const glm::vec3& position) const noexcept
{
    glm::vec3 relative = getRelativePosition(position);

    float32_t halfW = m_halfSize.x;
    float32_t halfH = m_halfSize.y;

    bool insideXY = (-halfW <= relative.x && relative.x <= halfW) &&
                    (-halfH <= relative.y && relative.y <= halfH);

    if (m_halfDepth <= 0.0f)
        return insideXY;

    return insideXY && (-m_halfDepth <= relative.z && relative.z <= m_halfDepth);
}

bool Field::contains(const Particle& particle) const noexcept
{
    return contains(particle.getPosition());
}

glm::vec3 Field::getRelativePosition(const glm::vec3& position) const noexcept
{
    return position - m_position;
}
