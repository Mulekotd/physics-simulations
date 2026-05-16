#pragma once

#include <functional>
#include <optional>
#include <utility>
#include <vector>

#include <glm/vec3.hpp>

#include "app/Constants.hpp"

#include "physics/Field.hpp"
#include "physics/Particle.hpp"

#include "physics/ForceFunctions.hpp"

namespace Simulation {
    using ForceFunc = std::function<void(Particle&, float)>;

    class Motion {
    public:
        Motion(std::size_t particleCount, Field& world);

        void setIntegrator(ForceFunc integrator) { m_integrator = std::move(integrator); }

        void render();
        void update(float dt);

        std::optional<std::size_t> pickParticle(const glm::vec3& origin, const glm::vec3& direction) const;
        std::optional<std::size_t> pickParticle2D(const glm::vec3& worldPoint) const;
        
        Particle* getParticleMutable(std::size_t index) noexcept;

        const Particle* getParticle(std::size_t index) const noexcept;
        const std::vector<Particle>& particles() const noexcept { return m_particles; }

    private:
        std::vector<Particle> m_particles;
        Field&                m_field;
        ForceFunc             m_forceGen;
        ForceFunc             m_integrator;

        void resolveBounds(Particle&) const;
        void resolveParticleCollision(Particle& a, Particle& b) const;
    };
}
