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
        inline constexpr float DEFAULT_3D_START_DISTANCE = 520.0f;
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
        inline constexpr float DRAG_FOLLOW_STIFFNESS = 18.0f;
        inline constexpr float DRAG_SCROLL_DEPTH_STEP = 90.0f;
        inline constexpr float DRAG_MIN_DEPTH = 40.0f;
        inline constexpr float FREE_FALL_INITIAL_3D_SPEED = 42.0f;
        inline constexpr float WINDOW_SHAKE_FORCE_SCALE = 120.0f;
        inline constexpr float WINDOW_SHAKE_FORCE_DECAY = 3.5f;
        inline constexpr float WINDOW_SHAKE_MAX_ACCELERATION = 320.0f;
        inline constexpr float ORBIT_GRAVITATIONAL_CONSTANT = 28.0f;
        inline constexpr float ORBIT_CENTER_MASS = 12000.0f;
        inline constexpr float ORBIT_CENTER_MASS_PER_PARTICLE = 180.0f;
        inline constexpr float ORBIT_SOFTENING = 140.0f;
        inline constexpr float ORBIT_SECONDARY_GRAVITY_MASS_RATIO = 0.18f;
        inline constexpr float ORBIT_SPIN_FACTOR = 2.4f;
        inline constexpr float ORBIT_CENTER_SPIN = 0.35f;
        inline constexpr float ORBIT_INITIAL_SPEED_SCALE = 1.0f;
        inline constexpr float ORBIT_INITIAL_SPEED_MAX = 180.0f;
    }

    namespace Particles {
        inline constexpr float MASS_MIN = 250.0f;
        inline constexpr float MASS_MAX = 2000.0f;
    }

    namespace Simulation {
        inline constexpr float FIELD_DEPTH_2D = 0.0f;
        inline constexpr float FIELD_DEPTH_3D = 3200.0f;
        inline constexpr float CLUSTER_RADIUS = 50.0f;
        inline constexpr float CLUSTER_MARGIN = 20.0f;
        inline constexpr int PARTICLE_COUNT_MIN = 25;
        inline constexpr int PARTICLE_COUNT_MAX = 500;
    }

    namespace Lighting {
        inline constexpr int MAX_LIGHTS = 8;
        inline constexpr float DEFAULT_RADIUS = 12.0f;
        inline constexpr float DEFAULT_RANGE = 900.0f;
        inline constexpr float MIN_RANGE = 80.0f;
        inline constexpr float MAX_RANGE = 3600.0f;
        inline constexpr float DEFAULT_INTENSITY = 0.28f;
    }
}
