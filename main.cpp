#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" 
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" 
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

// THE SHADERS
const char* vertexShaderSource = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec3 VertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Transform vertex position into world space
    FragPos = vec3(model * vec4(aPos, 1.0));
    // Transform normal vectors to world space correctly (handles non-uniform scaling)
    Normal = mat3(transpose(inverse(model))) * aNormal;
    VertexColor = aColor;

    // Final screen position with perspective projection
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 VertexColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
    // 1. Ambient lighting
    float ambientStrength = 0.25;
    vec3 ambient = ambientStrength * lightColor;

    // 2. Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 3. Specular lighting (highlight sheen)
    float specularStrength = 0.6;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // Combine all lighting components with vertex color
    vec3 result = (ambient + diffuse + specular) * VertexColor;
    FragColor = vec4(result, 1.0);
}
)";

// 3D CUBE GEOMETRY
float vertices[] = {
    // Positions          // Normals           // Colors 
    // Front
    -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.40f, 0.05f,
     0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.40f, 0.05f,
     0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.40f, 0.05f,
     0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.40f, 0.05f,
    -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.40f, 0.05f,
    -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.40f, 0.05f,

    // Back
    -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.85f, 0.15f, 0.10f,
     0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.85f, 0.15f, 0.10f,
     0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.85f, 0.15f, 0.10f,
     0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.85f, 0.15f, 0.10f,
    -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.85f, 0.15f, 0.10f,
    -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.85f, 0.15f, 0.10f,

    // Left
    -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.65f, 0.10f,
    -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.65f, 0.10f,
    -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.65f, 0.10f,
    -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.65f, 0.10f,
    -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.65f, 0.10f,
    -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.65f, 0.10f,

    // Right
     0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.50f, 0.10f,
     0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.50f, 0.10f,
     0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.50f, 0.10f,
     0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.50f, 0.10f,
     0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.50f, 0.10f,
     0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.50f, 0.10f,

    // Top
    -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.80f, 0.20f,
     0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.80f, 0.20f,
     0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.80f, 0.20f,
    -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.80f, 0.20f,
    -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.80f, 0.20f,
     0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.80f, 0.20f,

    // Bottom
    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.50f, 0.10f, 0.05f,
     0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.50f, 0.10f, 0.05f,
     0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.50f, 0.10f, 0.05f,
    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.50f, 0.10f, 0.05f,
     0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.50f, 0.10f, 0.05f,
    -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.50f, 0.10f, 0.05f
};

int main() {
    // Main window setup
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "GPU Benchmark", NULL, NULL);
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

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "========================================" << std::endl;
    std::cout << " GPU Vendor:   " << (vendor ? (const char*)vendor : "Unknown") << std::endl;
    std::cout << " GPU Renderer: " << (renderer ? (const char*)renderer : "Unknown") << std::endl;
    std::cout << " OpenGL Version:   " << (version ? (const char*)version : "Unknown") << std::endl;
    std::cout << "========================================" << std::endl;

    // Uncapped Framerate for benchmarking, 1 will cap to monitor framerate
    glfwSwapInterval(0);

    // 6. ENABLE DEPTH TESTING
    // Crucial for 3D: prevents back faces from rendering on top of front faces
    glEnable(GL_DEPTH_TEST);

    // Set background colour
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f);

    // COMPILE SHADERS
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkCompileErrors(shaderProgram, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Buffers
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLsizei stride = 9 * sizeof(float);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Colour
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Uniforms
    glUseProgram(shaderProgram);
    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    int projLoc = glGetUniformLocation(shaderProgram, "projection");
    int lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    int lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

    // Set static light values
    glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); // Pure white light
    glUniform3f(lightPosLoc, 2.0f, 3.0f, 3.0f);   // Light positioned up, right, and front

    glm::vec3 cameraPos(0.0f, 0.0f, 3.5f);
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

    double lastTime = glfwGetTime();
    int frameCount = 0;
    std::string gpuString = renderer ? (const char*)renderer : "GPU";

    // ==================================================================
    // 11. MAIN RENDER LOOP
    // ==================================================================
    while (!glfwWindowShouldClose(window)) {
        // Telemetry update
        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - lastTime >= 0.25) { // Update title every quarter second
            double fps = frameCount / (currentTime - lastTime);
            double frameTimeMs = 1000.0 / fps;

            std::string title = "GPU Benchmark | " + gpuString + 
                                " | FPS: " + std::to_string((int)fps) + 
                                " | Frame Time: " + std::to_string(frameTimeMs).substr(0, 5) + " ms";
            glfwSetWindowTitle(window, title.c_str());

            frameCount = 0;
            lastTime = currentTime;
        }

        // Clear buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        // Camera and Transformations
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        if (display_h == 0) display_h = 1; // Prevent division by zero

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)display_w / (float)display_h, 0.1f, 100.0f);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 model = glm::mat4(1.0f);
        float time = (float)glfwGetTime();
        model = glm::rotate(model, time * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.2f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // --- DRAW THE 3D CUBE ---
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
      
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}