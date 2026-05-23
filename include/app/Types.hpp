#pragma once

#include <cstdint>
#include <type_traits>

using float32_t = float;

static_assert(sizeof(float32_t) == 4, "float32_t must be 32 bits.");
static_assert(sizeof(std::uint32_t) == 4, "std::uint32_t must be 32 bits.");
static_assert(sizeof(std::int32_t) == 4, "std::int32_t must be 32 bits.");
static_assert(std::is_floating_point_v<float32_t>, "float32_t must be a floating-point type.");
