#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <unordered_map>

#include <glm/geometric.hpp>

#include "app/Application.hpp"
#include "app/Constants.hpp"
#include "physics/Motion.hpp"

using namespace Simulation;

Motion::Motion(std::size_t n, Field& world) : m_field(world)
{
    m_particles.reserve(n);

    glm::vec3 center = m_field.getPosition();
    glm::vec2 halfSize = m_field.getHalfSize();

    std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> angle_distribuition(0.f, 2.f * Constants::Math::PI);
    float clusterRadius = Constants::Simulation::CLUSTER_RADIUS;
    float maxClusterRadius = std::max(0.0f, std::min(halfSize.x, halfSize.y) - Constants::Simulation::CLUSTER_MARGIN);
    clusterRadius = std::min(clusterRadius, maxClusterRadius);

    std::uniform_real_distribution<float> radius_distribuition(0.f, clusterRadius);
    std::uniform_real_distribution<float> mass_distribuition(Constants::Particles::MASS_MIN, Constants::Particles::MASS_MAX);

    for (std::size_t i = 0; i < n; ++i)
    {
        float mass = mass_distribuition(rng);
        float radius = std::cbrt(mass) * 0.3f;

        glm::vec3 position = center;

        auto canPlaceAt = [&](const glm::vec3& candidate)
        {
            if (std::abs(candidate.x - center.x) + radius > halfSize.x) return false;
            if (std::abs(candidate.y - center.y) + radius > halfSize.y) return false;

            for (const auto& other : m_particles)
            {
                glm::vec3 delta = other.getPosition() - candidate;
                float minDist = other.getRadius() + radius;
                if (glm::dot(delta, delta) < minDist * minDist)
                    return false;
            }

            return true;
        };

        bool placed = false;

        constexpr int maxAttempts = 128;
        for (int attempt = 0; attempt < maxAttempts; ++attempt)
        {
            float r = std::sqrt(radius_distribuition(rng) / std::max(clusterRadius, 1.0f)) * clusterRadius;
            float ang = angle_distribuition(rng); // cluster angle

            glm::vec3 candidate{ center.x + r * std::cos(ang), center.y - r * std::sin(ang), 0.f };

            if (canPlaceAt(candidate))
            {
                position = candidate;
                placed = true;
                break;
            }
        }

        if (!placed)
        {
            float spacing = std::max(radius * 2.0f, 1.0f);
            for (float r = 0.0f; r <= maxClusterRadius && !placed; r += spacing)
            {
                int samples = std::max(8, static_cast<int>(std::ceil(2.0f * Constants::Math::PI * std::max(r, spacing) / spacing)));

                for (int sample = 0; sample < samples; ++sample)
                {
                    float ang = (2.0f * Constants::Math::PI * sample) / samples;
                    glm::vec3 candidate{ center.x + r * std::cos(ang), center.y + r * std::sin(ang), 0.f };

                    if (canPlaceAt(candidate))
                    {
                        position = candidate;
                        placed = true;
                        break;
                    }
                }
            }
        }

        glm::vec3 velocity{ 0.f, 0.f, 0.f }; // free fall → v0 = 0

        m_particles.emplace_back(position, velocity, mass, radius);
    }

    // forces initialization
    auto gravity = Forces::gravity(m_field);
    auto friction = Forces::friction(m_field);

    m_forceGen = [gravity, friction](Particle& particle, float dt)
    {
        gravity(particle, dt);
        friction(particle, dt);
    };

    m_integrator = [](Particle& particle, float dt)
    {
        particle.integrate(dt); // symplectic Euler (default)
    };
}

void Motion::render()
{
    for (const auto& particle : m_particles)
        particle.draw();
}

std::optional<std::size_t> Motion::pickParticle(const glm::vec3& origin, const glm::vec3& direction) const
{
    float closestT = std::numeric_limits<float>::max();

    std::optional<std::size_t> closestIndex;

    for (std::size_t i = 0; i < m_particles.size(); ++i)
    {
        const Particle& particle = m_particles[i];

        glm::vec3 oc = origin - particle.getPosition();

        float b = glm::dot(oc, direction);
        float c = glm::dot(oc, oc) - particle.getRadius() * particle.getRadius();
        float h = b * b - c;

        if (h < 0.0f)
            continue;

        float sqrtH = std::sqrt(h);
        float t = -b - sqrtH;

        if (t < 0.0f)
            t = -b + sqrtH;

        if (t >= 0.0f && t < closestT)
        {
            closestT = t;
            closestIndex = i;
        }
    }

    return closestIndex;
}

std::optional<std::size_t> Motion::pickParticle2D(const glm::vec3& worldPoint) const
{
    float closestDistSq = std::numeric_limits<float>::max();
    std::optional<std::size_t> closestIndex;

    for (std::size_t i = 0; i < m_particles.size(); ++i)
    {
        const Particle& particle = m_particles[i];

        glm::vec3 delta = particle.getPosition() - worldPoint;

        float distSq = glm::dot(delta, delta);
        float radius = particle.getRadius();

        if (distSq <= radius * radius && distSq < closestDistSq)
        {
            closestDistSq = distSq;
            closestIndex = i;
        }
    }

    return closestIndex;
}

Particle* Motion::getParticleMutable(std::size_t index) noexcept
{
    if (index >= m_particles.size())
        return nullptr;

    return &m_particles[index];
}

const Particle* Motion::getParticle(std::size_t index) const noexcept
{
    if (index >= m_particles.size())
        return nullptr;

    return &m_particles[index];
}

void Motion::update(float dt)
{
    // apply external forces + integrate
    for (auto& particle : m_particles)
    {
        if (m_forceGen)
            m_forceGen(particle, dt);

        if (m_integrator)
            m_integrator(particle, dt); // semi‑implicit

        resolveBounds(particle); // bounce off walls
    }

    const std::size_t count = m_particles.size();

    if (count == 0)
        return;

    float maxRadius = 0.0f;

    for (const auto& particle : m_particles)
        maxRadius = std::max(maxRadius, particle.getRadius());

    const float cellSize = std::max(1.0f, maxRadius * 2.0f);
    const glm::vec2 halfSize = m_field.getHalfSize();

    struct CellKey {
        int x;
        int y;

        bool operator==(const CellKey& other) const noexcept
        {
            return x == other.x && y == other.y;
        }
    };

    struct CellKeyHash {
        std::size_t operator()(const CellKey& key) const noexcept
        {
            return (static_cast<std::size_t>(static_cast<unsigned int>(key.x)) << 32) ^
                   static_cast<std::size_t>(static_cast<unsigned int>(key.y));
        }
    };

    auto toCell = [&](const glm::vec3& position)
    {
        glm::vec3 relative = m_field.getRelativePosition(position);

        float x = relative.x + halfSize.x;
        float y = relative.y + halfSize.y;

        int cx = static_cast<int>(std::floor(x / cellSize));
        int cy = static_cast<int>(std::floor(y / cellSize));

        return CellKey{ cx, cy };
    };

    std::unordered_map<CellKey, std::vector<std::size_t>, CellKeyHash> grid;

    grid.reserve(count);

    for (std::size_t i = 0; i < count; ++i)
        grid[toCell(m_particles[i].getPosition())].push_back(i);

    for (std::size_t i = 0; i < count; ++i)
    {
        const CellKey cell = toCell(m_particles[i].getPosition());

        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                CellKey neighbor{ cell.x + dx, cell.y + dy };
            
                auto it = grid.find(neighbor);
                if (it == grid.end())
                    continue;

                for (std::size_t j : it->second)
                {
                    if (j <= i) continue;
                    resolveParticleCollision(m_particles[i], m_particles[j]);
                }
            }
        }
    }
}

// ------------------------------------------------------------------
// Collision with world bounds
// ------------------------------------------------------------------
void Motion::resolveBounds(Particle& a) const
{
    const float restitution = m_field.getRestitutionConstant();

    glm::vec3 rel = m_field.getRelativePosition(a.getPosition());
    glm::vec3 vel = a.getVelocity();

    float halfW = m_field.getSize().x * 0.5f;
    float halfH = m_field.getSize().y * 0.5f;
    float radius = a.getRadius();

    bool hit = false;

    if (rel.x - radius < -halfW)
    {
        rel.x = -halfW + radius;
        vel.x = -vel.x * restitution;
        hit = true;
    }
    else if (rel.x + radius > halfW)
    {
        rel.x = halfW - radius;
        vel.x = -vel.x * restitution;
        hit = true;
    }

    if (rel.y - radius < -halfH)
    {
        rel.y = -halfH + radius;
        vel.y = -vel.y * restitution;
        hit = true;
    }
    else if (rel.y + radius > halfH)
    {
        rel.y = halfH - radius;
        vel.y = -vel.y * restitution;
        hit = true;
    }

    if (hit)
    {
        a.setPosition(rel + m_field.getPosition());
        a.setVelocity(vel);
    }
}

// ------------------------------------------------------------------
// Elastic collision between two discs
// ------------------------------------------------------------------
void Motion::resolveParticleCollision(Particle& a, Particle& b) const
{
    glm::vec3 delta = b.getPosition() - a.getPosition();

    float distanceSq = glm::dot(delta, delta);
    float radiusSum = a.getRadius() + b.getRadius();

    // no contact
    if (distanceSq >= radiusSum * radiusSum)
        return;

    float dist = std::sqrt(distanceSq);

    glm::vec3 n;

    float penetration = radiusSum - dist;

    if (dist < Constants::Math::EPSILON)
    {
        n = glm::vec3(1.f, 0.f, 0.f);
        penetration = radiusSum;
    } else
    {
        // collision normal
        n = delta * (1.0f / dist);
    }

    float invMa = 1.0f / a.getMass();
    float invMb = 1.0f / b.getMass();
    float invMassSum = invMa + invMb;

    a.setPosition(a.getPosition() - n * (penetration * invMa / invMassSum));
    b.setPosition(b.getPosition() + n * (penetration * invMb / invMassSum));

    // --- 2.2 impulse to change velocities
    glm::vec3 relVel = b.getVelocity() - a.getVelocity();
    float vRelN = glm::dot(relVel, n);

    // already separating
    if (vRelN >= 0.0f)
        return;

    float e = m_field.getRestitutionConstant(); // 0 = inelastic, 1 = elastic
    float j = -(1 + e) * vRelN / invMassSum;

    glm::vec3 impulse = n * j;

    a.setVelocity(a.getVelocity() - impulse * invMa);
    b.setVelocity(b.getVelocity() + impulse * invMb);
}
