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

namespace {
    bool hasDepth(const Field& field) noexcept
    {
        return field.getHalfDepth() > Constants::Math::EPSILON;
    }

    float randomDepthOffset(std::mt19937& rng, float halfDepth, float radius)
    {
        float usableHalfDepth = std::max(0.0f, halfDepth - radius);
        if (usableHalfDepth <= Constants::Math::EPSILON)
            return 0.0f;

        std::uniform_real_distribution<float> depthDistribution(-usableHalfDepth, usableHalfDepth);
        return depthDistribution(rng);
    }

    glm::vec3 randomOrbitDirection(std::mt19937& rng, std::uniform_real_distribution<float>& angleDistribution)
    {
        std::uniform_real_distribution<float> zDistribution(-1.0f, 1.0f);

        float z = zDistribution(rng);
        float xy = std::sqrt(std::max(0.0f, 1.0f - z * z));
        float angle = angleDistribution(rng);

        return { xy * std::cos(angle), xy * std::sin(angle), z };
    }

    glm::vec3 orbitTangentFor(const glm::vec3& radial)
    {
        glm::vec3 tangent = glm::cross(glm::vec3{ 0.f, 0.f, 1.f }, radial);

        if (glm::dot(tangent, tangent) <= Constants::Math::EPSILON)
            tangent = glm::cross(glm::vec3{ 0.f, 1.f, 0.f }, radial);

        return glm::normalize(tangent);
    }
}

Motion::Motion(std::size_t n, Field& world, Mode mode)
    : m_field(world),
      m_mode(mode)
{
    if (m_mode == Mode::Orbit)
        initializeOrbit(n);
    else
        initializeFreeFall(n);

    if (m_mode == Mode::Orbit)
    {
        m_integrator = [](Particle& particle, float dt)
        {
            particle.integrateSymplectic(dt);
        };
    }
    else
    {
        m_integrator = [](Particle& particle, float dt)
        {
            particle.integrate(dt);
        };
    }
}

void Motion::initializeFreeFall(std::size_t n)
{
    m_particles.reserve(n);

    glm::vec3 center = m_field.getPosition();
    m_orbitAnchor = center;
    glm::vec2 halfSize = m_field.getHalfSize();
    float halfDepth = m_field.getHalfDepth();
    bool useDepth = hasDepth(m_field);

    std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> angle_distribuition(0.f, 2.f * Constants::Math::PI);
    std::uniform_real_distribution<float> unitDistribution(0.f, 1.f);
    std::uniform_real_distribution<float> zDirectionDistribution(-1.f, 1.f);
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
            if (useDepth && std::abs(candidate.z - center.z) + radius > halfDepth) return false;

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
            glm::vec3 candidate = center;
            float effectiveClusterRadius = useDepth
                                               ? std::min(clusterRadius, std::max(0.0f, halfDepth - radius))
                                               : clusterRadius;

            if (useDepth)
            {
                float sphereRadius = std::cbrt(unitDistribution(rng)) * effectiveClusterRadius;
                float zDirection = zDirectionDistribution(rng);
                float xy = std::sqrt(std::max(0.0f, 1.0f - zDirection * zDirection));
                float ang = angle_distribuition(rng);

                candidate += glm::vec3{ sphereRadius * xy * std::cos(ang),
                                         sphereRadius * xy * std::sin(ang),
                                         sphereRadius * zDirection };
            }
            else
            {
                float r = std::sqrt(radius_distribuition(rng) / std::max(clusterRadius, 1.0f)) * clusterRadius;
                float ang = angle_distribuition(rng); // cluster angle
                candidate += glm::vec3{ r * std::cos(ang), -r * std::sin(ang), 0.f };
            }

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
                    float remainingSphereDepth = std::sqrt(std::max(0.0f, clusterRadius * clusterRadius - r * r));
                    float z = useDepth ? randomDepthOffset(rng, std::min(halfDepth, remainingSphereDepth), radius) : 0.0f;
                    glm::vec3 candidate{ center.x + r * std::cos(ang), center.y + r * std::sin(ang), center.z + z };

                    if (canPlaceAt(candidate))
                    {
                        position = candidate;
                        placed = true;
                        break;
                    }
                }
            }
        }

        glm::vec3 velocity{ 0.f, 0.f, 0.f };

        if (useDepth)
        {
            std::uniform_real_distribution<float> speedDistribution(-Constants::Physics::FREE_FALL_INITIAL_3D_SPEED,
                                                                    Constants::Physics::FREE_FALL_INITIAL_3D_SPEED);
            velocity = { speedDistribution(rng) * 0.35f,
                         0.f,
                         speedDistribution(rng) };
        }

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

}

void Motion::initializeOrbit(std::size_t n)
{
    m_particles.reserve(n);

    glm::vec3 center = m_field.getPosition();
    m_orbitAnchor = center;
    glm::vec2 halfSize = m_field.getHalfSize();
    bool useDepth = hasDepth(m_field);
    float maxOrbitRadius = std::max(80.0f, std::min(halfSize.x, halfSize.y) - Constants::Simulation::CLUSTER_MARGIN);
    float minOrbitRadius = std::min(90.0f, maxOrbitRadius * 0.35f);

    float centerMass = Constants::Physics::ORBIT_CENTER_MASS +
                       static_cast<float>(n) * Constants::Physics::ORBIT_CENTER_MASS_PER_PARTICLE;
    float centerRadius = std::cbrt(centerMass) * 0.35f;
    m_particles.emplace_back(center,
                             glm::vec3{ 0.f, 0.f, 0.f },
                             centerMass,
                             centerRadius,
                             Constants::Physics::ORBIT_CENTER_SPIN);

    if (n <= 1)
        return;

    std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> angleDistribution(0.f, 2.f * Constants::Math::PI);
    std::uniform_real_distribution<float> radiusDistribution(minOrbitRadius, maxOrbitRadius);
    std::uniform_real_distribution<float> massDistribution(Constants::Particles::MASS_MIN * 0.25f,
                                                           Constants::Particles::MASS_MAX * 0.45f);

    for (std::size_t i = 1; i < n; ++i)
    {
        float orbitRadius = radiusDistribution(rng);
        float mass = massDistribution(rng);
        float radius = std::cbrt(mass) * 0.22f;

        glm::vec3 radial;
        if (useDepth)
        {
            radial = randomOrbitDirection(rng, angleDistribution);
        }
        else
        {
            float angle = angleDistribution(rng);
            radial = { std::cos(angle), std::sin(angle), 0.f };
        }

        glm::vec3 tangent = useDepth
                                ? orbitTangentFor(radial)
                                : glm::vec3{ -radial.y, radial.x, 0.f };

        glm::vec3 position = center + radial * orbitRadius;
        float softeningSq = Constants::Physics::ORBIT_SOFTENING * Constants::Physics::ORBIT_SOFTENING;
        float radiusSq = orbitRadius * orbitRadius;
        float softenedDistanceSq = radiusSq + softeningSq;
        float softenedDistance = std::sqrt(softenedDistanceSq);
        float circularSpeedSq = Constants::Physics::ORBIT_GRAVITATIONAL_CONSTANT *
                                centerMass * radiusSq /
                                std::max(softenedDistanceSq * softenedDistance, 1.0f);
        float speed = std::sqrt(std::max(circularSpeedSq, 0.0f));
        speed = std::min(speed * Constants::Physics::ORBIT_INITIAL_SPEED_SCALE,
                         Constants::Physics::ORBIT_INITIAL_SPEED_MAX);
        glm::vec3 velocity = tangent * speed;
        float angularVelocity = (speed / std::max(orbitRadius, 1.0f)) * Constants::Physics::ORBIT_SPIN_FACTOR;

        m_particles.emplace_back(position, velocity, mass, radius, angularVelocity);
    }
}

void Motion::applyOrbitalForces()
{
    if (m_particles.size() < 2)
        return;

    for (std::size_t attractorIndex = 0; attractorIndex < m_particles.size(); ++attractorIndex)
    {
        const Particle& attractor = m_particles[attractorIndex];
        if (attractorIndex != 0 &&
            attractor.getMass() < m_particles.front().getMass() * Constants::Physics::ORBIT_SECONDARY_GRAVITY_MASS_RATIO)
        {
            continue;
        }

        for (std::size_t i = 1; i < m_particles.size(); ++i)
        {
            if (i == attractorIndex)
                continue;

            if (m_pinnedParticle.has_value() && *m_pinnedParticle == i)
                continue;

            Particle& particle = m_particles[i];
            glm::vec3 delta = attractor.getPosition() - particle.getPosition();
            float softeningSq = Constants::Physics::ORBIT_SOFTENING * Constants::Physics::ORBIT_SOFTENING;
            float softenedDistanceSq = glm::dot(delta, delta) + softeningSq;
            float softenedDistance = std::sqrt(softenedDistanceSq);

            if (softenedDistance <= Constants::Math::EPSILON)
                continue;

            float forceScale = Constants::Physics::ORBIT_GRAVITATIONAL_CONSTANT *
                               attractor.getMass() * particle.getMass() /
                               (softenedDistanceSq * softenedDistance);

            particle.addForce(delta * forceScale);
        }
    }
}

void Motion::render()
{
    if (Application::cameraMode == Application::CameraMode::ThreeD)
    {
        std::vector<const Particle*> drawOrder;
        drawOrder.reserve(m_particles.size());

        for (const auto& particle : m_particles)
            drawOrder.push_back(&particle);

        std::sort(drawOrder.begin(), drawOrder.end(),
                  [](const Particle* a, const Particle* b)
                  {
                      float depthA = glm::dot(a->getPosition() - Application::camera3D.getPosition(),
                                              Application::camera3D.getForward());
                      float depthB = glm::dot(b->getPosition() - Application::camera3D.getPosition(),
                                              Application::camera3D.getForward());
                      return depthA > depthB;
                  });

        for (const Particle* particle : drawOrder)
            particle->draw();

        return;
    }

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
    glm::vec3 transientAcceleration = m_transientAcceleration;
    m_transientAcceleration = { 0.f, 0.f, 0.f };

    if (m_mode == Mode::Orbit && !m_particles.empty())
    {
        if (m_pinnedParticle.has_value() && *m_pinnedParticle == 0)
        {
            glm::vec3 newAnchor = m_particles.front().getPosition();
            glm::vec3 anchorDelta = newAnchor - m_orbitAnchor;
            m_orbitAnchor = m_particles.front().getPosition();

            if (glm::dot(anchorDelta, anchorDelta) > Constants::Math::EPSILON)
            {
                for (std::size_t i = 1; i < m_particles.size(); ++i)
                    m_particles[i].setPosition(m_particles[i].getPosition() + anchorDelta);
            }
        }
        else
            m_particles.front().setPosition(m_orbitAnchor);

        m_particles.front().setVelocity({ 0.f, 0.f, 0.f });
        m_particles.front().clearForces();
    }

    if (m_mode == Mode::Orbit)
        applyOrbitalForces();

    // apply external forces + integrate
    for (std::size_t i = 0; i < m_particles.size(); ++i)
    {
        if (m_mode == Mode::Orbit && i == 0)
        {
            m_particles[i].setPosition(m_orbitAnchor);
            m_particles[i].setVelocity({ 0.f, 0.f, 0.f });
            m_particles[i].clearForces();
            if (m_integrator)
                m_integrator(m_particles[i], dt);
            continue;
        }

        if (m_pinnedParticle.has_value() && *m_pinnedParticle == i)
        {
            m_particles[i].clearForces();
            continue;
        }

        Particle& particle = m_particles[i];

        if (m_forceGen)
            m_forceGen(particle, dt);

        if (m_mode == Mode::FreeFall && glm::dot(transientAcceleration, transientAcceleration) > Constants::Math::EPSILON)
            particle.addForce(transientAcceleration * particle.getMass());

        if (m_integrator)
            m_integrator(particle, dt); // semi‑implicit

        if (m_mode != Mode::Orbit)
            resolveBounds(particle); // bounce off walls
    }

    const std::size_t count = m_particles.size();

    if (count == 0)
        return;

    if (m_mode == Mode::Orbit)
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
                    if (m_mode == Mode::Orbit && (i == 0 || j == 0)) continue;
                    if (m_pinnedParticle.has_value() && (*m_pinnedParticle == i || *m_pinnedParticle == j)) continue;
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
    float halfD = m_field.getHalfDepth();
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

    if (halfD > 0.0f)
    {
        if (rel.z - radius < -halfD)
        {
            rel.z = -halfD + radius;
            vel.z = -vel.z * restitution;
            hit = true;
        }
        else if (rel.z + radius > halfD)
        {
            rel.z = halfD - radius;
            vel.z = -vel.z * restitution;
            hit = true;
        }
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
