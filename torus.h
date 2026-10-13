#ifndef TORUS_H
#define TORUS_H

#include <vector>
#include <cmath>
#include <glm/glm.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct TorusVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 color;
};

// makes a torus (donut) mesh
inline void generateTorus(float mainRadius, float tubeRadius, int rings, int segments,
                          std::vector<TorusVertex>& outVertices,
                          std::vector<unsigned int>& outIndices) {
    outVertices.clear();
    outIndices.clear();

    outVertices.reserve((rings + 1) * (segments + 1));
    outIndices.reserve(rings * segments * 6);

    for (int i = 0; i <= rings; ++i) {
        float u = (float)i / (float)rings * 2.0f * (float)M_PI;
        float cosU = std::cos(u);
        float sinU = std::sin(u);

        for (int j = 0; j <= segments; ++j) {
            float v = (float)j / (float)segments * 2.0f * (float)M_PI;
            float cosV = std::cos(v);
            float sinV = std::sin(v);

            TorusVertex vert;
            vert.position = glm::vec3(
                (mainRadius + tubeRadius * cosV) * cosU,
                tubeRadius * sinV,
                (mainRadius + tubeRadius * cosV) * sinU
            );

            // normal points out from the tube
            vert.normal = glm::normalize(glm::vec3(
                cosV * cosU,
                sinV,
                cosV * sinU
            ));

            // uvs for the fur
            vert.texCoords = glm::vec2(
                (float)i / (float)rings * 4.0f,
                (float)j / (float)segments * 2.0f
            );

            // orange gradient
            float factor = (cosV + 1.0f) * 0.5f;
            vert.color = glm::mix(glm::vec3(0.95f, 0.25f, 0.05f), glm::vec3(1.0f, 0.70f, 0.15f), factor);

            outVertices.push_back(vert);
        }
    }

    // 2 triangles per quad
    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < segments; ++j) {
            int current = i * (segments + 1) + j;
            int next = (i + 1) * (segments + 1) + j;

            outIndices.push_back(current);
            outIndices.push_back(next);
            outIndices.push_back(current + 1);

            outIndices.push_back(next);
            outIndices.push_back(next + 1);
            outIndices.push_back(current + 1);
        }
    }
}

#endif // TORUS_H
