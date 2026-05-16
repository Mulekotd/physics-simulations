#pragma once

#include <cstdint>
#include <functional>

class Particle;

using Id = std::uint32_t;
using ForceFunc = std::function<void(Particle&, float)>;
