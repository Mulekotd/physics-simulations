#pragma once

#include <cstdint>

#include "common/Vector3.hpp"

struct Dimensions {
    int width, height, depth = 0;

    constexpr Dimensions(int w, int h) : width(w), height(h) {}
    constexpr Dimensions(int w, int h, int d) : width(w), height(h), depth(d) {}

    Vector3 toVector() const noexcept {
        return { static_cast<float>(width), static_cast<float>(height), static_cast<float>(depth) };
    }

    constexpr Dimensions center() const noexcept {
        return { width / 2, height / 2, depth / 2 };
    }

    Vector3 centerAsVector() const noexcept {
        return toVector() * 0.5f;
    }
};
