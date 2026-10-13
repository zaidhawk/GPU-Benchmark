#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <algorithm>
#include <iostream>
#include <string>

#include "benchmark.h"
#include "gpu_monitor.h"
#include "hud.h"
#include "render_target.h"
#include "scenes.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// live stats shown in the title bar
std::string windowTitle(const std::string& gpuName, const GPUMonitor& gpu, double fps, const Resolution& res) {
    std::string title = "GPUBench | " + gpuName;
    if (gpu.isAvailable) {
        title += " | " + std::to_string(gpu.temperatureC) + "C | GPU: " + std::to_string(gpu.utilizationGpu) + "%";
    }
    title += " | FPS: " + std::to_string((int)fps) +
             " | Res: " + res.name + " (" + std::to_string(res.width) + " x " + std::to_string(res.height) + ")";
    return title;
}

int main() {
    // window + opengl setup
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "GPUBench", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }
    glfwSwapInterval(0); // vsync off so fps isnt capped
    glEnable(GL_DEPTH_TEST);

    GPUMonitor gpuMonitor;
    const char* rendererStr = (const char*)glGetString(GL_RENDERER);
    std::string gpuName = rendererStr ? rendererStr : "Unknown GPU";
    size_t slash = gpuName.find('/'); // nvidia adds "/PCIe/SSE2" to the name, cut it off
    if (slash != std::string::npos) gpuName = gpuName.substr(0, slash);

    // imgui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr; // dont save imgui.ini
    hud::loadFonts();
    hud::applyStyle();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    Scenes scenes;
    scenes.init();

    int scene = SCENE_CUBE;
    int resIndex = 0;       // selected in the dropdown
    int activeResIndex = 0; // what the render target is using right now
    RenderTarget target;
    target.create(resolutions[resIndex].width, resolutions[resIndex].height);

    Benchmark bench;

    double prevTime = glfwGetTime();
    int fpsFrames = 0;
    double fpsTimer = 0.0;
    double fps = 0.0;
    float shownUtil = 0.0f; // smoothed so the bar doesnt jump

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (resIndex != activeResIndex) {
            activeResIndex = resIndex;
            target.create(resolutions[activeResIndex].width, resolutions[activeResIndex].height);
        }

        double now = glfwGetTime();
        double deltaTime = std::max(now - prevTime, 0.00001);
        prevTime = now;

        // update fps, temp/usage and title every 0.3s
        fpsFrames++;
        fpsTimer += deltaTime;
        if (fpsTimer >= 0.3) {
            fps = fpsFrames / fpsTimer;
            gpuMonitor.update();
            glfwSetWindowTitle(window, windowTitle(gpuName, gpuMonitor, fps, resolutions[activeResIndex]).c_str());
            fpsFrames = 0;
            fpsTimer = 0.0;
        }
        float ease = std::min(1.0f, (float)deltaTime * 8.0f);
        shownUtil += ((float)gpuMonitor.utilizationGpu - shownUtil) * ease;

        bench.update(now, scene, activeResIndex);

        // draw the scene offscreen then copy it to the window
        target.bind();
        scenes.draw(scene, (float)now, target.aspect());

        int winW, winH;
        glfwGetFramebufferSize(window, &winW, &winH);
        target.blitToWindow(winW, winH);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        hud::draw(gpuName, fps, gpuMonitor, shownUtil, scene, resIndex, bench, glfwGetTime());
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // cleanup
    gpuMonitor.shutdown();
    target.destroy();
    scenes.destroy();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
