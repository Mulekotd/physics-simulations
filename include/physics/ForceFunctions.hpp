#pragma once

#include <algorithm>

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

#include "app/Constants.hpp"

#include "physics/Field.hpp"
#include "physics/Particle.hpp"

namespace Simulation::Forces {
    // Uniform near-surface gravitational field: F = m * g.
    inline auto gravity(const Field& world)
    {
        return [&world](Particle& particle, float)
        {
            float g = world.getGravityConstant();
            particle.addForce(glm::vec3{0, -1, 0} * g * particle.getMass());
        };
    }

    // dynamic friction
    inline auto friction(const Field& world)
    {
        return [&world](Particle& particle, float dt)
        {
            float g  = world.getGravityConstant();
            float mu = world.getFrictionConstant();

            const glm::vec3& velocity = particle.getVelocity();

            float speed = glm::length(velocity);

            if (speed > 0.0001f)
            {
                glm::vec3 direction = -glm::normalize(velocity);
                float kineticFriction = mu * particle.getMass() * g;
                float maxStoppingForce = particle.getMass() * speed / std::max(dt, Constants::Physics::MIN_DT);
                glm::vec3 force = direction * std::min(kineticFriction, maxStoppingForce);

                particle.addForce(force);
            }
        };
    }
}
