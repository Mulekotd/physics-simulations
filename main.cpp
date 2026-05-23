#include "app/Application.hpp"

int main()
{
    if (!Application::Init())
        return -1;

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(Application::window))
    {
        double currentTime = glfwGetTime();
        float32_t deltaTime = static_cast<float32_t>(currentTime - lastTime);

        lastTime = currentTime;

        Application::Tick(deltaTime);

        glfwPollEvents();
    }

    Application::Cleanup();

    return 0;
}
