#include "common/Application.hpp"

int main() {
    if (!Application::Init()) return -1;

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(Application::window)) {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);

        lastTime = currentTime;

        Application::Tick(deltaTime);

        glfwPollEvents();
    }

    Application::Cleanup();

    return 0;
}
