#pragma once

#include "common/Constants.hpp"
#include "common/Vector3.hpp"

#include "core/Field.hpp"
#include "core/Particle.hpp"

namespace Simulation::Forces {
    // weight force
    inline auto gravity(const Field& world) {
        return [&world](Particle& particle, float) {
            float g = world.getGravityConstant();
            particle.addForce(Vector3{0, -1, 0} * g * particle.getMass());
        };
    }

    // dynamic friction
    inline auto friction(const Field& world) {
        return [&world](Particle& particle, float) {
            float g  = world.getGravityConstant();
            float mu = world.getFrictionConstant();

            const Vector3& velocity = particle.getVelocity();
            float speed = velocity.length();

            if (speed > 0.0001f) {
                Vector3 direction = -velocity.normalized();
                Vector3 force = direction * mu * particle.getMass() * g;

                particle.addForce(force);
            }
        };
    }
}
