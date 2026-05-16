#pragma once

namespace Constants {
    namespace Math {
        inline constexpr float PI = 3.14159265358979323846f;
        inline constexpr float EPSILON = 1e-5f;
    }

    namespace Window {
        inline constexpr int DEFAULT_WIDTH = 800;
        inline constexpr int DEFAULT_HEIGHT = 600;
        inline constexpr const char* DEFAULT_TITLE = "Physics Simulation";
    }

    namespace Camera {
        inline constexpr float ZOOM_MIN = 1.0f;
        inline constexpr float ZOOM_MAX = 2.0f;
        inline constexpr float DEFAULT_3D_MOVE_SPEED = 200.0f;
        inline constexpr float DEFAULT_3D_MOUSE_SENSITIVITY = 0.08f;
        inline constexpr float DEFAULT_3D_FOV_RADIANS = 1.0472f;
    }

    namespace Physics {
        inline constexpr float GRAVITY = 9.81f;
        inline constexpr float RESTITUTION_COEFFICIENT = 0.8f;
        inline constexpr float DYNAMIC_FRICTION_COEFFICIENT = 0.05f;
        inline constexpr float MIN_MASS = 1e-4f;
        inline constexpr float MIN_DT = 1e-4f;
        inline constexpr float MAX_FRAME_TIME = 1.0f / 30.0f;
        inline constexpr float FIXED_TIME_STEP = 1.0f / 120.0f;
        inline constexpr float MAX_DRAG_SPEED = 2000.0f;
    }

    namespace Particles {
        inline constexpr float MASS_MIN = 250.0f;
        inline constexpr float MASS_MAX = 2000.0f;
    }

    namespace Simulation {
        inline constexpr float CLUSTER_RADIUS = 50.0f;
        inline constexpr float CLUSTER_MARGIN = 20.0f;
        inline constexpr int PARTICLE_COUNT_MIN = 25;
        inline constexpr int PARTICLE_COUNT_MAX = 500;
    }
}
