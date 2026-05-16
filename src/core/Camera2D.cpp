#include <algorithm>

#include "common/Constants.hpp"
#include "common/Dimensions.hpp"
#include "common/Vector3.hpp"

#include "core/Field.hpp"
#include "core/Camera2D.hpp"

void Camera2D::setZoom(float factor) noexcept {
    if (factor > 0.0f) {
        m_zoom = std::clamp(m_zoom * factor, Constants::ZOOM_MIN, Constants::ZOOM_MAX);
        clampOffset();
    }
}

void Camera2D::move(const Vector3& delta) noexcept {
    m_offset.x += delta.x;
    m_offset.y += delta.y;

    clampOffset();
}

Vector3 Camera2D::worldToNDC(const Vector3& position) const noexcept {
    Vector3 relative = m_field->getRelativePosition(position) - m_offset;
    const Dimensions& size = m_field->getSize();

    float x = (relative.x / (size.width  * 0.5f)) * m_zoom;
    float y = (relative.y / (size.height * 0.5f)) * m_zoom;

    return { x, y, 0.f };
}

void Camera2D::clampOffset() {
    const Dimensions& size = m_field->getSize();

    float halfW = size.width  * 0.5f;
    float halfH = size.height * 0.5f;

    float visHalfW = halfW / m_zoom;
    float visHalfH = halfH / m_zoom;

    float maxOffX = std::max(0.f, halfW - visHalfW);
    float maxOffY = std::max(0.f, halfH - visHalfH);

    m_offset.x = std::clamp(m_offset.x, -maxOffX,  maxOffX);
    m_offset.y = std::clamp(m_offset.y, -maxOffY,  maxOffY);
}
