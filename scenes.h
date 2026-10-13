#ifndef SCENES_H
#define SCENES_H

// the 3 scenes: cube, fur torus, armada

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <cmath>
#include <string>
#include <vector>

#include "shaders.h"
#include "cube.h"
#include "torus.h"

enum Scene { SCENE_CUBE, SCENE_TORUS, SCENE_ARMADA };
inline const char* sceneNames[] = { "Cube", "Torus", "Armada" };

// fixed settings so scores can be compared
const int   FUR_SHELLS     = 64;     // fur layers on the torus
const float FUR_LENGTH     = 0.22f;
const float FUR_DENSITY    = 45.0f;
const int   ARMADA_COUNT   = 5000;   // cubes in the armada
const float ARMADA_SPREAD  = 7.0f;
const float ROTATION_SPEED = 45.0f;  // degrees/sec

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
};

// 8 lights moving around the scene
inline std::array<PointLight, 8> orbitingLights(float t) {
    return {{
        { glm::vec3(sin(t * 1.1f) * 4.0f, cos(t * 0.8f) * 2.2f, cos(t * 1.1f) * 4.0f), glm::vec3(1.0f, 0.40f, 0.05f) },
        { glm::vec3(cos(t * 1.3f) * 4.5f, sin(t * 1.5f) * 2.5f, sin(t * 1.3f) * 4.5f), glm::vec3(0.1f, 0.80f, 1.00f) },
        { glm::vec3(sin(t * 0.9f) * 4.8f, 1.8f, cos(t * 0.9f) * 4.8f),                 glm::vec3(1.0f, 0.15f, 0.15f) },
        { glm::vec3(cos(t * 0.7f) * 4.2f, -1.8f, sin(t * 0.7f) * 4.2f),                glm::vec3(1.0f, 0.85f, 0.10f) },
        { glm::vec3(sin(t * 1.4f) * 3.5f, cos(t * 1.2f) * 3.2f, sin(t * 0.6f) * 3.5f), glm::vec3(0.8f, 0.15f, 1.00f) },
        { glm::vec3(cos(t * 1.0f) * 3.8f, sin(t * 0.9f) * 3.4f, cos(t * 1.5f) * 3.8f), glm::vec3(0.15f, 1.0f, 0.45f) },
        { glm::vec3(sin(t * 1.7f) * 4.6f, -1.2f, cos(t * 1.7f) * 4.6f),                glm::vec3(1.0f, 0.20f, 0.85f) },
        { glm::vec3(cos(t * 2.0f) * 3.0f, 2.2f, sin(t * 2.0f) * 3.0f),                 glm::vec3(1.0f, 1.00f, 1.00f) }
    }};
}

// helpers for setting shader uniforms
inline void setInt(unsigned int program, const char* name, int v)                { glUniform1i(glGetUniformLocation(program, name), v); }
inline void setFloat(unsigned int program, const char* name, float v)            { glUniform1f(glGetUniformLocation(program, name), v); }
inline void setVec3(unsigned int program, const char* name, const glm::vec3& v)  { glUniform3fv(glGetUniformLocation(program, name), 1, glm::value_ptr(v)); }
inline void setMat4(unsigned int program, const char* name, const glm::mat4& m)  { glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, glm::value_ptr(m)); }

struct Scenes {
    unsigned int cubeProgram = 0, furProgram = 0, armadaProgram = 0;
    unsigned int cubeVAO = 0, cubeVBO = 0;
    unsigned int torusVAO = 0, torusVBO = 0, torusEBO = 0;
    GLsizei torusIndexCount = 0;

    void init() {
        cubeProgram   = createProgram(cubeVertexShader, litFragmentShader);
        furProgram    = createProgram(furVertexShader, furFragmentShader);
        armadaProgram = createProgram(armadaVertexShader, litFragmentShader);
        uploadCube();
        uploadTorus();
    }

    void draw(int scene, float t, float aspect) {
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 300.0f);
        auto lights = orbitingLights(t);

        if (scene == SCENE_CUBE) {
            glm::vec3 cameraPos(0.0f, 1.2f, 3.8f);
            useProgram(cubeProgram, projection, cameraPos, lights);
            setMat4(cubeProgram, "model", spin(t, glm::vec3(0.5f, 1.0f, 0.2f)));

            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        } else if (scene == SCENE_TORUS) {
            glm::vec3 cameraPos(0.0f, 2.2f, 5.8f);
            useProgram(furProgram, projection, cameraPos, lights);
            setMat4(furProgram, "model", spin(t, glm::vec3(0.4f, 1.0f, 0.2f)));
            setInt(furProgram, "totalLayers", FUR_SHELLS);
            setFloat(furProgram, "furLength", FUR_LENGTH);
            setFloat(furProgram, "furDensity", FUR_DENSITY);

            // all fur layers in one instanced draw call
            glBindVertexArray(torusVAO);
            glDrawElementsInstanced(GL_TRIANGLES, torusIndexCount, GL_UNSIGNED_INT, 0, FUR_SHELLS);
        } else {
            glm::vec3 cameraPos(0.0f, 4.0f, ARMADA_SPREAD * 2.2f);
            useProgram(armadaProgram, projection, cameraPos, lights);
            setFloat(armadaProgram, "time", t);
            setInt(armadaProgram, "totalInstances", ARMADA_COUNT);
            setFloat(armadaProgram, "spread", ARMADA_SPREAD);

            // all the cubes in one instanced draw call
            glBindVertexArray(cubeVAO);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 36, ARMADA_COUNT);
        }
    }

    void destroy() {
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteBuffers(1, &cubeVBO);
        glDeleteVertexArrays(1, &torusVAO);
        glDeleteBuffers(1, &torusVBO);
        glDeleteBuffers(1, &torusEBO);
        glDeleteProgram(cubeProgram);
        glDeleteProgram(furProgram);
        glDeleteProgram(armadaProgram);
    }

private:
    // rotation based on time
    static glm::mat4 spin(float t, const glm::vec3& axis) {
        return glm::rotate(glm::mat4(1.0f), t * glm::radians(ROTATION_SPEED), axis);
    }

    // binds the shader and sets camera + lights
    static void useProgram(unsigned int program, const glm::mat4& projection, const glm::vec3& cameraPos,
                           const std::array<PointLight, 8>& lights) {
        glUseProgram(program);
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        setMat4(program, "projection", projection);
        setMat4(program, "view", view);
        setVec3(program, "viewPos", cameraPos);

        setInt(program, "activeLights", (int)lights.size());
        for (size_t i = 0; i < lights.size(); ++i) {
            std::string base = "lights[" + std::to_string(i) + "]";
            setVec3(program, (base + ".position").c_str(), lights[i].position);
            setVec3(program, (base + ".color").c_str(), lights[i].color);
        }
    }

    // send cube vertices to the gpu
    void uploadCube() {
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);

        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

        GLsizei stride = 9 * sizeof(float);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);                   // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float))); // normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float))); // color
        glEnableVertexAttribArray(2);
    }

    // build the torus mesh and send it to the gpu
    void uploadTorus() {
        std::vector<TorusVertex> vertices;
        std::vector<unsigned int> indices;
        generateTorus(1.8f, 0.7f, 64, 32, vertices, indices);
        torusIndexCount = (GLsizei)indices.size();

        glGenVertexArrays(1, &torusVAO);
        glGenBuffers(1, &torusVBO);
        glGenBuffers(1, &torusEBO);

        glBindVertexArray(torusVAO);
        glBindBuffer(GL_ARRAY_BUFFER, torusVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TorusVertex), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torusEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        GLsizei stride = sizeof(TorusVertex);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(TorusVertex, position));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(TorusVertex, normal));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(TorusVertex, texCoords));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(TorusVertex, color));
        glEnableVertexAttribArray(3);
    }
};

#endif // SCENES_H
