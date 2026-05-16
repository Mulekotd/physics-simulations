#include "common/Vector3.hpp"

#include "core/Field.hpp"
#include "core/Particle.hpp"

Field::Field(Dimensions size, float gravity, float friction, float restitution)
    : m_size(size),
      m_position(size.centerAsVector()),
      m_gravity(gravity),
      m_friction(friction),
      m_restitution(restitution) 
{} // TODO: group physics constants in a single struct

void Field::reset() {
    m_gravity = Constants::Physics::GRAVITY;
    m_friction = Constants::Physics::DYNAMIC_FRICTION_COEFFICIENT;
    m_restitution = Constants::Physics::RESTITUTION_COEFFICIENT;
    m_position = m_size.centerAsVector();
}

bool Field::contains(const Vector3& position) const noexcept {
    Vector3 relative = getRelativePosition(position);

    float halfW = m_size.width  * 0.5f;
    float halfH = m_size.height * 0.5f;

    return (-halfW <= relative.x && relative.x <= halfW) &&
           (-halfH <= relative.y && relative.y <= halfH);
}

bool Field::contains(const Particle& particle) const noexcept {
    return contains(particle.getPosition());
}

Vector3 Field::getRelativePosition(const Vector3& position) const noexcept {
    return position - m_position;
}
