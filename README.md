# Physics Simulation

## 📌 Overview

A C++ physics simulation project using modern OpenGL (GLFW) for real-time visualization of particles and fields. The system is modular, supporting extensible simulation laws (Newton's Laws), and integrates Dear ImGui for interactive UI controls.

## 🛠️ Selected Technologies

### 1. C++ 17

The main programming language, chosen for its performance and modern features, enabling efficient simulation and extensible architecture.

<img src="https://upload.wikimedia.org/wikipedia/commons/1/18/ISO_C%2B%2B_Logo.svg" width="64" height="64" alt="C++ Logo"/>

---

### 2. OpenGL (GLFW)

Used for rendering the simulation in real time, handling window/context creation, and drawing particles and fields.

<img src="https://opengl.org/img/opengl_logo.png" width="64" height="64" alt="OpenGL Logo"/>

<img src="https://cdn-b.saashub.com/images/app/service_logos/38/b48cc85cebb2/large.png?1553244024" width="64" height="64" alt="GLFW Logo"/>

---

### 3. CMake

Build system generator for cross-platform compilation and easy dependency management.

<img src="https://upload.wikimedia.org/wikipedia/commons/1/13/Cmake.svg" width="64" height="64" alt="CMake Logo"/>

---

### 4. ImGui

Immediate Mode GUI library for C++ to provide interactive controls and visualization.

![ImGui](https://blog.conan.io/assets/post_images/2019-06-26/conan-imgui-triangle-rotate-color.gif)

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

On Ubuntu/Debian:

```bash
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libgl1-mesa-dev
```

### 3. Build the Project

```bash
mkdir build
cd build
cmake ..
make
```

### 4. Run the Simulation

```bash
./physics-simulation
```

## 🤝 Feedback and Contributions

Feedback, suggestions, and contributions are welcome!  
If you have ideas for improvements or encounter any issues, please [open an issue](https://github.com/Mulekotd/physics-simulation/issues) on GitHub.
