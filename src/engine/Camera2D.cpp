#include <algorithm>

#include "app/Constants.hpp"

#include "physics/Field.hpp"
#include "engine/Camera2D.hpp"

void Camera2D::setZoom(float factor) noexcept
{
    if (factor > 0.0f)
    {
        m_zoom = std::clamp(m_zoom * factor, Constants::Camera::ZOOM_MIN, Constants::Camera::ZOOM_MAX);
        clampOffset();
    }
}

void Camera2D::move(const glm::vec3& delta) noexcept
{
    m_offset.x += delta.x;
    m_offset.y += delta.y;

    clampOffset();
}

glm::vec3 Camera2D::worldToNDC(const glm::vec3& position) const noexcept
{
    glm::vec3 relative = m_field->getRelativePosition(position) - m_offset;
    const glm::vec2& size = m_field->getSize();

    float x = (relative.x / (size.x * 0.5f)) * m_zoom;
    float y = (relative.y / (size.y * 0.5f)) * m_zoom;

    return { x, y, 0.f };
}

glm::vec3 Camera2D::ndcToWorld(const glm::vec3& ndc) const noexcept
{
    const glm::vec2& size = m_field->getSize();

    float halfW = size.x * 0.5f;
    float halfH = size.y * 0.5f;

    glm::vec3 relative{ ndc.x * (halfW / m_zoom), ndc.y * (halfH / m_zoom), 0.f };

    return m_field->getPosition() + m_offset + relative;
}

void Camera2D::clampOffset()
{
    const glm::vec2& size = m_field->getSize();

    float halfW = size.x * 0.5f;
    float halfH = size.y * 0.5f;

    float visHalfW = halfW / m_zoom;
    float visHalfH = halfH / m_zoom;

    float maxOffX = std::max(0.f, halfW - visHalfW);
    float maxOffY = std::max(0.f, halfH - visHalfH);

    m_offset.x = std::clamp(m_offset.x, -maxOffX,  maxOffX);
    m_offset.y = std::clamp(m_offset.y, -maxOffY,  maxOffY);
}

void Camera2D::reset() noexcept {
    m_zoom = 1.f;
    m_offset = { 0.f, 0.f, 0.f };
}
