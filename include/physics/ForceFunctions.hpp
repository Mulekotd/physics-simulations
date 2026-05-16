#pragma once

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

#include "app/Constants.hpp"

#include "physics/Field.hpp"
#include "physics/Particle.hpp"

namespace Simulation::Forces {
    // weight force
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
        return [&world](Particle& particle, float)
        {
            float g  = world.getGravityConstant();
            float mu = world.getFrictionConstant();

            const glm::vec3& velocity = particle.getVelocity();

            float speed = glm::length(velocity);

            if (speed > 0.0001f)
            {
                glm::vec3 direction = -glm::normalize(velocity);
                glm::vec3 force = direction * mu * particle.getMass() * g;

                particle.addForce(force);
            }
        };
    }
}
