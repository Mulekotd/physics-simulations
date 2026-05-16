#include <random>

#include "common/Application.hpp"
#include "common/Constants.hpp"
#include "common/Dimensions.hpp"
#include "common/Types.hpp"

#include "simulation/Motion.hpp"

using namespace Simulation;

Motion::Motion(std::size_t n, Field& world) : m_field(world) {
    m_particles.reserve(n);

    Vector3 center = m_field.getPosition();

    std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> angle_distribuition(0.f, 2.f * Constants::Math::PI);
    std::uniform_real_distribution<float> radius_distribuition(0.f, Constants::Simulation::CLUSTER_RADIUS);
    std::uniform_real_distribution<float> mass_distribuition(Constants::Random::MASS_MIN, Constants::Random::MASS_MAX);

    for (std::size_t i = 0; i < n; ++i) {
        float r = radius_distribuition(rng);  // cluster radius
        float ang = angle_distribuition(rng); // cluster angle

        Vector3 position{ center.x + r * std::cos(ang), center.y - r * std::sin(ang), 0.f };
        Vector3 velocity{ 0.f, 0.f, 0.f }; // free fall → v0 = 0

        float mass = mass_distribuition(rng);
        float radius = std::cbrt(mass) * 0.3f;

        m_particles.emplace_back(position, velocity, mass, radius);
    }

    // forces initialization

    auto gravity = Forces::gravity(m_field);
    auto friction = Forces::friction(m_field);

    m_forceGen = [gravity, friction](Particle& particle, float dt) {
        gravity(particle, dt);
        friction(particle, dt);
    };
}

void Motion::render() {
    for (const auto& particle : m_particles) {
        particle.draw();
    }
}

void Motion::update(float dt) {
    // apply external forces + integrate
    for (auto& particle : m_particles) {
        if (m_forceGen) m_forceGen(particle, dt);

        particle.integrate(dt);  // semi‑implicit
        resolveBounds(particle); // bounce off walls
    }

    const std::size_t count = m_particles.size();
    
    // O(n²) pair‑wise collision
    for (std::size_t i = 0; i < count; ++i)
        for (std::size_t j = i + 1; j < count; ++j)
            resolveParticleCollision(m_particles[i], m_particles[j]);
}

// ------------------------------------------------------------------
// Collision with world bounds
// ------------------------------------------------------------------
void Motion::resolveBounds(Particle& a) const {
    const float restitution = m_field.getRestitutionConstant();

    Vector3 rel = m_field.getRelativePosition(a.getPosition());
    Vector3 vel = a.getVelocity();

    float halfW = m_field.getSize().width  * 0.5f;
    float halfH = m_field.getSize().height * 0.5f;

    bool hit = false;

    if (rel.x < -halfW) { rel.x = -halfW; vel.x = -vel.x * restitution; hit = true; }
    else if (rel.x > halfW) { rel.x =  halfW; vel.x = -vel.x * restitution; hit = true; }

    if (rel.y < -halfH) { rel.y = -halfH; vel.y = -vel.y * restitution; hit = true; }
    else if (rel.y > halfH) { rel.y =  halfH; vel.y = -vel.y * restitution; hit = true; }

    if (hit) {
        a.setPosition(rel + m_field.getPosition());
        a.setVelocity(vel);
    }
}

// ------------------------------------------------------------------
// Elastic collision between two discs
// ------------------------------------------------------------------
void Motion::resolveParticleCollision(Particle& a, Particle& b) const {
    Vector3 delta = b.getPosition() - a.getPosition();

    float distanceSq = delta.dot(delta);
    float radiusSum = a.getRadius() + b.getRadius();

    // no contact
    if (distanceSq >= radiusSum * radiusSum) return;

    float dist = std::sqrt(distanceSq);

    // same position
    if (dist == 0.0f) return;                              

    // --- 2.1 push the discs apart (positional correction)
    Vector3 n = delta * (1.0f / dist); // collision normal

    float penetration = radiusSum - dist;

    float invMa = 1.0f / a.getMass();
    float invMb = 1.0f / b.getMass();
    float invMassSum = invMa + invMb;

    a.setPosition(a.getPosition() - n * (penetration * invMa / invMassSum));
    b.setPosition(b.getPosition() + n * (penetration * invMb / invMassSum));

    // --- 2.2 impulse to change velocities
    Vector3 relVel = b.getVelocity() - a.getVelocity();
    float vRelN = relVel.dot(n);

    // already separating
    if (vRelN >= 0.0f) return;

    float e = m_field.getRestitutionConstant(); // 0 = inelastic, 1 = elastic
    float j = -(1 + e) * vRelN / invMassSum;

    Vector3 impulse = n * j;
    a.setVelocity(a.getVelocity() - impulse * invMa);
    b.setVelocity(b.getVelocity() + impulse * invMb);
}
