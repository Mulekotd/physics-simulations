#pragma once

#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "app/Constants.hpp"
#include "engine/Ray.hpp"

class Camera3D {
public:
    Camera3D(glm::vec3 position = { 0.f, 0.f, 5.f }, float yaw = -90.f, float pitch = 0.f);

    [[nodiscard]] const glm::vec3& getPosition() const noexcept { return m_position; }
    [[nodiscard]] float getYaw() const noexcept { return m_yaw; }
    [[nodiscard]] float getPitch() const noexcept { return m_pitch; }

    [[nodiscard]] const glm::vec3& getForward() const noexcept { return m_forward; }
    [[nodiscard]] const glm::vec3& getRight() const noexcept { return m_right; }
    [[nodiscard]] const glm::vec3& getUp() const noexcept { return m_up; }

    [[nodiscard]] float getMoveSpeed() const noexcept { return m_moveSpeed; }
    [[nodiscard]] float getMouseSensitivity() const noexcept { return m_mouseSensitivity; }
    [[nodiscard]] float getFovRadians() const noexcept { return m_fovRadians; }

    void setPosition(const glm::vec3& position) noexcept { m_position = position; }
    void setMoveSpeed(float speed) noexcept { m_moveSpeed = speed; }
    void setMouseSensitivity(float sensitivity) noexcept { m_mouseSensitivity = sensitivity; }
    void setFovRadians(float fovRadians) noexcept { m_fovRadians = fovRadians; }

    void move(const glm::vec3& delta) noexcept { m_position += delta; }
    void processMouseDelta(float deltaX, float deltaY) noexcept;

    [[nodiscard]] glm::mat4 getViewMatrix() const noexcept;
    [[nodiscard]] glm::mat4 getProjectionMatrix(float aspect) const noexcept;
    [[nodiscard]] glm::vec3 worldToNDC(const glm::vec3& position, const glm::vec2& viewport) const noexcept;
    [[nodiscard]] bool isSphereInFront(const glm::vec3& position, float radius) const noexcept;
    [[nodiscard]] Ray screenPointToRay(double x, double y, const glm::vec2& viewport) const noexcept;

private:
    void updateVectors() noexcept;

    glm::vec3 m_position;
    glm::vec3 m_forward{ 0.f, 0.f, -1.f };
    glm::vec3 m_right{ 1.f, 0.f, 0.f };
    glm::vec3 m_up{ 0.f, 1.f, 0.f };
    glm::quat m_orientation{ 1.f, 0.f, 0.f, 0.f };

    float m_yaw;
    float m_pitch;

    float m_moveSpeed = Constants::Camera::DEFAULT_3D_MOVE_SPEED;
    float m_mouseSensitivity = Constants::Camera::DEFAULT_3D_MOUSE_SENSITIVITY;
    float m_fovRadians = Constants::Camera::DEFAULT_3D_FOV_RADIANS;
    float m_nearPlane = 0.1f;
    float m_farPlane = 5000.f;
};
