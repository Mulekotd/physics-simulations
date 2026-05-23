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
        return [&world](Particle& particle, float32_t)
        {
            float32_t g = world.getGravityConstant();
            particle.addForce(glm::vec3{0, -1, 0} * g * particle.getMass());
        };
    }

    // dynamic friction
    inline auto friction(const Field& world)
    {
        return [&world](Particle& particle, float32_t dt)
        {
            float32_t g  = world.getGravityConstant();
            float32_t mu = world.getFrictionConstant();

            const glm::vec3& velocity = particle.getVelocity();

            float32_t speed = glm::length(velocity);

            if (speed > 0.0001f)
            {
                glm::vec3 direction = -glm::normalize(velocity);
                float32_t kineticFriction = mu * particle.getMass() * g;
                float32_t maxStoppingForce = particle.getMass() * speed / std::max(dt, Constants::Physics::MIN_DT);
                glm::vec3 force = direction * std::min(kineticFriction, maxStoppingForce);

                particle.addForce(force);
            }
        };
    }
}
