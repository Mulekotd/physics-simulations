#include <algorithm>
#include <cmath>

#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "engine/Camera3D.hpp"

Camera3D::Camera3D(glm::vec3 position, float yaw, float pitch)
    : m_position(position),
      m_yaw(yaw),
      m_pitch(pitch)
{
    updateVectors();
}

void Camera3D::processMouseDelta(float deltaX, float deltaY) noexcept
{
    m_yaw += deltaX * m_mouseSensitivity;
    m_pitch -= deltaY * m_mouseSensitivity;

    m_pitch = std::clamp(m_pitch, -89.f, 89.f);

    updateVectors();
}

glm::mat4 Camera3D::getViewMatrix() const noexcept
{
    return glm::lookAt(m_position, m_position + m_forward, m_up);
}

glm::mat4 Camera3D::getProjectionMatrix(float aspect) const noexcept
{
    return glm::perspective(m_fovRadians, aspect, m_nearPlane, m_farPlane);
}

glm::vec3 Camera3D::worldToNDC(const glm::vec3& position, const glm::vec2& viewport) const noexcept
{
    float aspect = viewport.y > 0.f ? (viewport.x / viewport.y) : 1.f;

    glm::mat4 proj = getProjectionMatrix(aspect);
    glm::mat4 view = getViewMatrix();

    glm::vec4 clip = proj * view * glm::vec4(position, 1.f);

    if (std::fabs(clip.w) < 1e-6f)
        return { 0.f, 0.f, 0.f };

    glm::vec3 ndc = glm::vec3(clip) / clip.w;

    return ndc;
}

bool Camera3D::isSphereInFront(const glm::vec3& position, float radius) const noexcept
{
    float depth = glm::dot(position - m_position, m_forward);
    return depth > m_nearPlane && depth - radius < m_farPlane;
}

Ray Camera3D::screenPointToRay(double x, double y, const glm::vec2& viewport) const noexcept
{
    if (viewport.x <= 0.f || viewport.y <= 0.f)
        return { m_position, m_forward };

    float ndcX = (2.f * static_cast<float>(x) / viewport.x) - 1.f;
    float ndcY = 1.f - (2.f * static_cast<float>(y) / viewport.y);

    glm::vec4 clip{ ndcX, ndcY, -1.f, 1.f };

    float aspect = viewport.y > 0.f ? (viewport.x / viewport.y) : 1.f;
    glm::mat4 invProj = glm::inverse(getProjectionMatrix(aspect));
    glm::vec4 eye = invProj * clip;
    eye = glm::vec4(eye.x, eye.y, -1.f, 0.f);

    glm::mat4 invView = glm::inverse(getViewMatrix());
    glm::vec3 dir = glm::normalize(glm::vec3(invView * eye));

    return { m_position, dir };
}

void Camera3D::updateVectors() noexcept
{
    const glm::vec3 worldUp{ 0.f, 1.f, 0.f };
    float yawRad = glm::radians(m_yaw + 90.f);
    float pitchRad = glm::radians(m_pitch);

    glm::quat yawRotation = glm::angleAxis(yawRad, worldUp);
    glm::quat pitchRotation = glm::angleAxis(pitchRad, glm::vec3{ 1.f, 0.f, 0.f });
    m_orientation = glm::normalize(yawRotation * pitchRotation);

    m_forward = glm::normalize(m_orientation * glm::vec3{ 0.f, 0.f, -1.f });
    m_right = glm::normalize(m_orientation * glm::vec3{ 1.f, 0.f, 0.f });
    m_up = glm::normalize(glm::cross(m_right, m_forward));
}
