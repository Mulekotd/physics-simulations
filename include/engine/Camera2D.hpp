#pragma once

#include <glm/vec3.hpp>

#include "app/Types.hpp"

class Field;

class Camera2D {
public:
    explicit Camera2D(const Field& world) : m_field(&world) {}

    [[nodiscard]] float32_t getZoom() const noexcept { return m_zoom; }
    [[nodiscard]] glm::vec3 getOffset() const noexcept { return m_offset; }

    void reset() noexcept;

    void setZoom(float32_t factor) noexcept;
    void move(const glm::vec3& delta) noexcept;

    glm::vec3 worldToNDC(const glm::vec3& position) const noexcept;
    glm::vec3 ndcToWorld(const glm::vec3& ndc) const noexcept;

private:
    const Field* m_field;
    float32_t        m_zoom = 1.f;
    glm::vec3    m_offset = { 0.f, 0.f, 0.f };

    void clampOffset(); 
};
