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
    using ForceFunc = std::function<void(Particle&, float32_t)>;

    enum class Mode {
        FreeFall,
        Orbit
    };

    class Motion {
    public:
        Motion(std::size_t particleCount, Field& world, Mode mode = Mode::FreeFall);

        void setIntegrator(ForceFunc integrator) { m_integrator = std::move(integrator); }
        void setPinnedParticle(std::optional<std::size_t> index) noexcept { m_pinnedParticle = index; }
        [[nodiscard]] bool isOrbitCenter(std::size_t index) const noexcept { return m_mode == Mode::Orbit && index == 0; }

        void render();
        void update(float32_t dt);
        void addTransientAcceleration(const glm::vec3& acceleration) noexcept { m_transientAcceleration += acceleration; }

        std::optional<std::size_t> pickParticle(const glm::vec3& origin, const glm::vec3& direction) const;
        std::optional<std::size_t> pickParticle2D(const glm::vec3& worldPoint) const;
        
        Particle* getParticleMutable(std::size_t index) noexcept;

        const Particle* getParticle(std::size_t index) const noexcept;
        const std::vector<Particle>& particles() const noexcept { return m_particles; }

    private:
        std::vector<Particle> m_particles;
        Field&                m_field;
        Mode                  m_mode;
        std::optional<std::size_t> m_pinnedParticle;
        glm::vec3             m_orbitAnchor{ 0.f, 0.f, 0.f };
        glm::vec3             m_transientAcceleration{ 0.f, 0.f, 0.f };
        ForceFunc             m_forceGen;
        ForceFunc             m_integrator;

        void initializeFreeFall(std::size_t particleCount);
        void initializeOrbit(std::size_t particleCount);
        void applyOrbitalForces();
        void resolveBounds(Particle&) const;
        void resolveParticleCollision(Particle& a, Particle& b) const;
    };
}
