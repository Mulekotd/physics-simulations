#include <cmath>

#include "common/Constants.hpp"
#include "common/Vector3.hpp"

// Constructors

Vector3::Vector3() : x(0), y(0), z(0) {}
Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

// Operators (+, -, *, /)

Vector3 Vector3::operator+(const Vector3& other) const {
    return Vector3(x + other.x, y + other.y, z + other.z);
}

Vector3 Vector3::operator-() const {
    return Vector3(-x, -y, -z);
}

Vector3 Vector3::operator-(const Vector3& other) const {
    return Vector3(x - other.x, y - other.y, z - other.z);
}

Vector3 Vector3::operator*(float scalar) const {
    return Vector3(x * scalar, y * scalar, z * scalar);
}

Vector3 Vector3::operator/(float scalar) const {
    return Vector3(x / scalar, y / scalar, z / scalar);
}

Vector3& Vector3::operator+=(const Vector3& other) {
    x += other.x;
    y += other.y;
    z += other.z;

    return *this;
}

Vector3& Vector3::operator-=(const Vector3& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;

    return *this;
}

Vector3& Vector3::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;

    return *this;
}

Vector3& Vector3::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    z /= scalar;

    return *this;
}

bool Vector3::operator==(const Vector3& other) const {
    return std::fabs(x - other.x) < Constants::Math::EPSILON &&
           std::fabs(y - other.y) < Constants::Math::EPSILON &&
           std::fabs(z - other.z) < Constants::Math::EPSILON;
}

bool Vector3::operator!=(const Vector3& other) const {
    return !(*this == other);
}

// Methods

float Vector3::distance(const Vector3& other) const {
    return (*this - other).length();
}

float Vector3::dot(const Vector3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

float Vector3::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vector3 Vector3::normalized() const {
    float len = length();

    if (len > 0) return Vector3(x / len, y / len, z / len);

    return Vector3(0, 0, 0);
}

Vector3 operator*(float scalar, const Vector3 other) {
    return Vector3(other.x * scalar, other.y * scalar, other.z * scalar);
}
