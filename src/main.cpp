#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <thread>

#include "wrappers/glfw.hpp"
#include "scene/Scene.hpp"
#include "scene/ImGuiS.hpp"
#include "resources/FileUtils.hpp"
#include "resources/Cache.hpp"

using HighClock = std::chrono::high_resolution_clock;

template <typename Rep, typename Period>
constexpr auto toFloatSeconds(const std::chrono::duration<Rep, Period>& dur)
{
    return std::chrono::duration_cast<std::chrono::duration<float>>(dur).count();
}

constexpr auto UpdatePeriod = std::chrono::microseconds(16666);
constexpr auto UpdatePeriodSeconds = toFloatSeconds(UpdatePeriod);

void enableOpenGLErrorHandler();

int main()
{
    // Init GLFW
    glfw::InitGuard initGuard;

    // Prepare the window we want to draw to
    glfw::WindowHint hint;
    hint.contextVersion(4, 5, glfw::Profile::Core);
    hint.resizable(false);
    hint.depthBits(0);
    hint.doublebuffer();
    hint.debugContext();

    // Create the window
    auto window = glfw::Window::borderless("INF584 Project", hint);

    // Make the window current
    window.makeCurrent();
    window.setSwapInterval(1);

    std::cout << "Using OpenGL " << window.getContextVersion().toString() << std::endl;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("Failed to initialize GLAD!");

    // Setup Dear Imgui
    scene::ImGuiGuard imGuiGuard(window);

#ifndef NDEBUG
    enableOpenGLErrorHandler();
#endif
    glEnable(GL_DEPTH_TEST); gl::checkError();
    glDepthFunc(GL_LEQUAL); gl::checkError();
    glEnable(GL_CULL_FACE); gl::checkError();
    glCullFace(GL_BACK); gl::checkError();
    glFrontFace(GL_CCW); gl::checkError();

    fileUtils::addDefaultLoaders();
    scene::Scene scene(window);

    auto then = HighClock::now();
    while (!window.shouldClose())
    {
        auto now = HighClock::now();

        while (now <= then)
        {
            std::this_thread::sleep_until(then);
            now = HighClock::now();
        }

        scene::beginImGui();
        std::size_t counter = 0;
        while (now > then)
        {
            then += UpdatePeriod;
            scene.update(UpdatePeriodSeconds);
        }

        scene.draw();
        scene::endImGui();
        window.swapBuffers();
        glfw::pollEvents();

        then = now;

        if (window.getKey(glfw::key::Escape))
            window.setShouldClose();
    }

    cache::clear();
}