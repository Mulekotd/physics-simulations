#pragma once

#include "app/Types.hpp"

namespace Constants {
    namespace Math {
        inline constexpr float32_t PI = 3.14159265358979323846f;
        inline constexpr float32_t EPSILON = 1e-5f;
    }

    namespace Window {
        inline constexpr std::int32_t DEFAULT_WIDTH = 800;
        inline constexpr std::int32_t DEFAULT_HEIGHT = 600;
        inline constexpr const char* DEFAULT_TITLE = "Physics Simulation";
    }

    namespace Camera {
        inline constexpr float32_t ZOOM_MIN = 1.0f;
        inline constexpr float32_t ZOOM_MAX = 2.0f;
        inline constexpr float32_t DEFAULT_3D_MOVE_SPEED = 200.0f;
        inline constexpr float32_t DEFAULT_3D_MOUSE_SENSITIVITY = 0.08f;
        inline constexpr float32_t DEFAULT_3D_FOV_RADIANS = 1.0472f;
        inline constexpr float32_t DEFAULT_3D_START_DISTANCE = 520.0f;
    }

    namespace Physics {
        inline constexpr float32_t GRAVITY = 9.81f;
        inline constexpr float32_t RESTITUTION_COEFFICIENT = 0.8f;
        inline constexpr float32_t DYNAMIC_FRICTION_COEFFICIENT = 0.05f;
        inline constexpr float32_t MIN_MASS = 1e-4f;
        inline constexpr float32_t MIN_DT = 1e-4f;
        inline constexpr float32_t MAX_FRAME_TIME = 1.0f / 30.0f;
        inline constexpr float32_t FIXED_TIME_STEP = 1.0f / 120.0f;
        inline constexpr float32_t MAX_DRAG_SPEED = 2000.0f;
        inline constexpr float32_t DRAG_FOLLOW_STIFFNESS = 18.0f;
        inline constexpr float32_t DRAG_SCROLL_DEPTH_STEP = 90.0f;
        inline constexpr float32_t DRAG_MIN_DEPTH = 40.0f;
        inline constexpr float32_t FREE_FALL_INITIAL_3D_SPEED = 42.0f;
        inline constexpr float32_t WINDOW_SHAKE_FORCE_SCALE = 120.0f;
        inline constexpr float32_t WINDOW_SHAKE_FORCE_DECAY = 3.5f;
        inline constexpr float32_t WINDOW_SHAKE_MAX_ACCELERATION = 320.0f;
        inline constexpr float32_t ORBIT_GRAVITATIONAL_CONSTANT = 28.0f;
        inline constexpr float32_t ORBIT_CENTER_MASS = 12000.0f;
        inline constexpr float32_t ORBIT_CENTER_MASS_PER_PARTICLE = 180.0f;
        inline constexpr float32_t ORBIT_SOFTENING = 140.0f;
        inline constexpr float32_t ORBIT_SECONDARY_GRAVITY_MASS_RATIO = 0.18f;
        inline constexpr float32_t ORBIT_SPIN_FACTOR = 2.4f;
        inline constexpr float32_t ORBIT_CENTER_SPIN = 0.35f;
        inline constexpr float32_t ORBIT_INITIAL_SPEED_SCALE = 1.0f;
        inline constexpr float32_t ORBIT_INITIAL_SPEED_MAX = 180.0f;
    }

    namespace Particles {
        inline constexpr float32_t MASS_MIN = 250.0f;
        inline constexpr float32_t MASS_MAX = 2000.0f;
    }

    namespace Simulation {
        inline constexpr float32_t FIELD_DEPTH_2D = 0.0f;
        inline constexpr float32_t FIELD_DEPTH_3D = 3200.0f;
        inline constexpr float32_t CLUSTER_RADIUS = 50.0f;
        inline constexpr float32_t CLUSTER_MARGIN = 20.0f;
        inline constexpr std::int32_t PARTICLE_COUNT_MIN = 25;
        inline constexpr std::int32_t PARTICLE_COUNT_MAX = 500;
    }

    namespace Lighting {
        inline constexpr std::int32_t MAX_LIGHTS = 8;
        inline constexpr float32_t DEFAULT_RADIUS = 12.0f;
        inline constexpr float32_t DEFAULT_RANGE = 900.0f;
        inline constexpr float32_t MIN_RANGE = 80.0f;
        inline constexpr float32_t MAX_RANGE = 3600.0f;
        inline constexpr float32_t DEFAULT_INTENSITY = 0.28f;
    }
}
