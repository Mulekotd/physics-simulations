#pragma once

#include "common/Vector3.hpp"

class Field;

class Camera2D {
public:
    explicit Camera2D(const Field& world) : m_field(&world) {}

    [[nodiscard]] float getZoom() const noexcept { return m_zoom; }
    [[nodiscard]] Vector3 getOffset() const noexcept { return m_offset; }

    void setZoom(float factor) noexcept;
    void move(const Vector3& delta) noexcept;
    Vector3 worldToNDC(const Vector3& position) const noexcept;

private:
    const Field* m_field;
    float        m_zoom = 1.f;
    Vector3      m_offset = { 0.f, 0.f, 0.f };

    void clampOffset(); 
};
