#pragma once

class Vector3 {
public:
    float x, y, z;

    Vector3();
    Vector3(float x, float y, float z);

    Vector3 operator+(const Vector3& other) const;
    Vector3 operator-() const; // Unary negation
    Vector3 operator-(const Vector3& other) const;
    Vector3 operator*(float scalar) const;
    Vector3 operator/(float scalar) const;

    Vector3& operator+=(const Vector3& other);
    Vector3& operator-=(const Vector3& other);
    Vector3& operator*=(float scalar);
    Vector3& operator/=(float scalar);

    bool operator==(const Vector3& other) const;
    bool operator!=(const Vector3& other) const;

    float distance(const Vector3& other) const;
    float dot(const Vector3& other) const;
    float length() const;
    
    Vector3 normalized() const;
};

Vector3 operator*(float scalar, const Vector3 other);
