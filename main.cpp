#include <glad/glad.h>  // GLAD must be included before GLFW!
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    // 1. Start up GLFW
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    // 2. Create the window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "GPU Benchmark", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 3. Tell GLFW to use this window
    glfwMakeContextCurrent(window);

    // 4. WAKE UP GLAD (The Translator)
    // We pass it a GLFW function that knows exactly where to find your GPU drivers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    // 5. SET THE STATE: Tell the GPU what color to use for the background (Dark Gray)
    // The numbers are Red, Green, Blue, and Alpha (Opacity). 0.0 is black, 1.0 is full color.
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // 6. The Main Loop
    while (!glfwWindowShouldClose(window)) {
        // EXECUTE THE COMMAND: Wipe the screen clean using the color we set above
        glClear(GL_COLOR_BUFFER_BIT);

        // Swap the front and back buffers
        glfwSwapBuffers(window);
        
        // Listen for hardware events (mouse, keyboard, close button)
        glfwPollEvents(); 
    }

    // 7. Clean up and close
    glfwTerminate();
    return 0;
}