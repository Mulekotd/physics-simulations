# Physics Simulations

## 📌 Overview

A C++ physics simulations project using modern OpenGL (GLFW) for real-time visualization of particles and fields. The system is modular, supporting extensible simulation laws (Newton's Laws), and integrates Dear ImGui for interactive UI controls.

## 🛠️ Selected Technologies

### 1. C++ 17

The main programming language, chosen for its performance and modern features, enabling efficient simulation and extensible architecture.

<img src="https://upload.wikimedia.org/wikipedia/commons/1/18/ISO_C%2B%2B_Logo.svg" width="64" height="64" alt="C++ Logo"/>

---

### 2. OpenGL (GLFW)

Used for rendering the simulation in real time, handling window/context creation, and drawing particles and fields.

<div style="display:flex;flex-direction:row;gap:1rem;"><img src="https://opengl.org/img/opengl_logo.png" width="128" height="72" alt="OpenGL Logo"/>
<img src="https://cdn-b.saashub.com/images/app/service_logos/38/b48cc85cebb2/large.png?1553244024" width="64" height="72" alt="GLFW Logo"/></div>

---

### 3. CMake

Build system generator for cross-platform compilation and easy dependency management.

<img src="https://upload.wikimedia.org/wikipedia/commons/1/13/Cmake.svg" width="64" height="64" alt="CMake Logo"/>

---

### 4. ImGui

Immediate Mode GUI library for C++ to provide interactive controls and visualization.

![ImGui](https://blog.conan.io/assets/post_images/2019-06-26/conan-imgui-triangle-rotate-color.gif)

---

### 5. GLM

Header-only math library used for vectors and linear algebra. It is fetched via CMake during configuration.

---

## ⚙️ Installation and Setup

### 1. Clone the Repository

This project uses Git submodules (e.g. Dear ImGui), so make sure to clone the repository recursively:

```bash
git clone --recursive https://github.com/Mulekotd/physics-simulation.git
cd physics-simulation
```

If you already cloned the repository without submodules, run:

```bash
git submodule update --init --recursive
```

### 2. Install Dependencies

Make sure you have the following installed on your system:

- C++17 compatible compiler (e.g., g++ 9+)
- CMake 3.14+
- OpenGL development libraries
- GLFW 3.x
- GLM (fetched automatically by CMake)

On Ubuntu/Debian:

```bash
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libgl1-mesa-dev
```

### 3. Build the Project

Debug build:

```bash
cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug --parallel
```

Release build:

```bash
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
cmake --build build/release --parallel
```

### 4. Run the Simulation

```bash
./physics-simulation
```

## 📁 Project Structure

```
physics-simulation/
├── CMakeLists.txt
├── LICENSE
├── README.md
├── imgui.ini
├── include
│   ├── app
│   │   ├── Application.hpp
│   │   ├── Constants.hpp
│   │   └── Types.hpp
│   ├── engine
│   │   ├── Camera2D.hpp
│   │   ├── Camera3D.hpp
│   │   ├── Ray.hpp
│   │   ├── ShaderProgram.hpp
│   │   └── TextureManager.hpp
│   ├── physics
│   │   ├── Field.hpp
│   │   ├── ForceFunctions.hpp
│   │   ├── Motion.hpp
│   │   └── Particle.hpp
│   └── ui
│       ├── ImGuiLayer.hpp
│       └── InputManager.hpp
├── main.cpp
├── shaders
│   ├── particle.frag
│   ├── particle.vert
│   ├── shadow_depth.frag
│   └── shadow_depth.vert
└── src
    ├── app
    │   └── Application.cpp
    ├── engine
    │   ├── Camera2D.cpp
    │   ├── Camera3D.cpp
    │   ├── ShaderProgram.cpp
    │   └── TextureManager.cpp
    ├── physics
    │   ├── Field.cpp
    │   ├── Motion.cpp
    │   └── Particle.cpp
    └── ui
        ├── ImGuiLayer.cpp
        └── InputManager.cpp
```

## 🤝 Feedback and Contributions

Feedback, suggestions, and contributions are welcome!  
If you have ideas for improvements or encounter any issues, please [open an issue](https://github.com/Mulekotd/physics-simulations/issues) on GitHub.
