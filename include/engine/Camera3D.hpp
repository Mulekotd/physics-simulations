#pragma once

#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "app/Constants.hpp"
#include "engine/Ray.hpp"

class Camera3D {
public:
    Camera3D(glm::vec3 position = { 0.f, 0.f, 5.f }, float32_t yaw = -90.f, float32_t pitch = 0.f);

    [[nodiscard]] const glm::vec3& getPosition() const noexcept { return m_position; }
    [[nodiscard]] float32_t getYaw() const noexcept { return m_yaw; }
    [[nodiscard]] float32_t getPitch() const noexcept { return m_pitch; }

    [[nodiscard]] const glm::vec3& getForward() const noexcept { return m_forward; }
    [[nodiscard]] const glm::vec3& getRight() const noexcept { return m_right; }
    [[nodiscard]] const glm::vec3& getUp() const noexcept { return m_up; }

    [[nodiscard]] float32_t getMoveSpeed() const noexcept { return m_moveSpeed; }
    [[nodiscard]] float32_t getMouseSensitivity() const noexcept { return m_mouseSensitivity; }
    [[nodiscard]] float32_t getFovRadians() const noexcept { return m_fovRadians; }

    void setPosition(const glm::vec3& position) noexcept { m_position = position; }
    void setMoveSpeed(float32_t speed) noexcept { m_moveSpeed = speed; }
    void setMouseSensitivity(float32_t sensitivity) noexcept { m_mouseSensitivity = sensitivity; }
    void setFovRadians(float32_t fovRadians) noexcept { m_fovRadians = fovRadians; }

    void move(const glm::vec3& delta) noexcept { m_position += delta; }
    void processMouseDelta(float32_t deltaX, float32_t deltaY) noexcept;

    [[nodiscard]] glm::mat4 getViewMatrix() const noexcept;
    [[nodiscard]] glm::mat4 getProjectionMatrix(float32_t aspect) const noexcept;
    [[nodiscard]] glm::vec3 worldToNDC(const glm::vec3& position, const glm::vec2& viewport) const noexcept;
    [[nodiscard]] bool isSphereInFront(const glm::vec3& position, float32_t radius) const noexcept;
    [[nodiscard]] Ray screenPointToRay(double x, double y, const glm::vec2& viewport) const noexcept;

private:
    void updateVectors() noexcept;

    glm::vec3 m_position;
    glm::vec3 m_forward{ 0.f, 0.f, -1.f };
    glm::vec3 m_right{ 1.f, 0.f, 0.f };
    glm::vec3 m_up{ 0.f, 1.f, 0.f };
    glm::quat m_orientation{ 1.f, 0.f, 0.f, 0.f };

    float32_t m_yaw;
    float32_t m_pitch;

    float32_t m_moveSpeed = Constants::Camera::DEFAULT_3D_MOVE_SPEED;
    float32_t m_mouseSensitivity = Constants::Camera::DEFAULT_3D_MOUSE_SENSITIVITY;
    float32_t m_fovRadians = Constants::Camera::DEFAULT_3D_FOV_RADIANS;
    float32_t m_nearPlane = 0.1f;
    float32_t m_farPlane = 5000.f;
};
