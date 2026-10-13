#ifndef SHADERS_H
#define SHADERS_H

// shader code + compile helpers

#include <glad/glad.h>
#include <iostream>

// cube vertex shader
inline const char* cubeVertexShader = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec3 VertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    VertexColor = aColor;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

// fur shader (idea from msi kombustor)
inline const char* furVertexShader = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec3 BaseColor;
out float LayerFraction;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int totalLayers;
uniform float furLength;

void main() {
    float layerFrac = float(gl_InstanceID) / float(max(totalLayers - 1, 1));
    LayerFraction = layerFrac;

    // push each layer out a bit along the normal
    vec3 displaced = aPos + aNormal * (layerFrac * furLength);

    FragPos = vec3(model * vec4(displaced, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    BaseColor = aColor;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

inline const char* furFragmentShader = R"(#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 BaseColor;
in float LayerFraction;

struct PointLight {
    vec3 position;
    vec3 color;
};

#define MAX_LIGHTS 8
uniform PointLight lights[MAX_LIGHTS];
uniform int activeLights;
uniform vec3 viewPos;
uniform float furDensity;

// cheap random number from a 2d position
float hash21(vec2 p) {
    p = fract(p * vec2(234.34, 435.345));
    p += dot(p, p + 34.23);
    return fract(p.x * p.y);
}

void main() {
    // throw away pixels that arent part of a hair
    if (LayerFraction > 0.0) {
        vec2 uv = TexCoords * furDensity;
        vec2 gId = floor(uv);
        vec2 gUv = fract(uv) - 0.5;

        float rnd = hash21(gId);
        float strandRadius = 0.20 + 0.26 * rnd;

        // hairs get thinner at the tips
        float currentMaxRadius = strandRadius * (1.0 - LayerFraction * 0.88);
        if (length(gUv) > currentMaxRadius) {
            discard;
        }
    }

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // dark at the roots, orange at the tips
    vec3 rootColor = BaseColor * 0.25;
    vec3 tipColor = vec3(1.0, 0.65, 0.15);
    vec3 furCol = mix(rootColor, tipColor, LayerFraction);

    vec3 totalLight = vec3(0.12); // ambient
    for (int i = 0; i < activeLights && i < MAX_LIGHTS; ++i) {
        vec3 lightDir = normalize(lights[i].position - FragPos);
        float dist = length(lights[i].position - FragPos);
        float attenuation = 1.0 / (1.0 + 0.08 * dist + 0.02 * dist * dist);

        // diffuse
        float diff = max(dot(norm, lightDir), 0.0);

        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);

        totalLight += (diff + spec * 0.7) * lights[i].color * attenuation;
    }

    FragColor = vec4(furCol * totalLight, 1.0);
}
)";

// armada vertex shader
inline const char* armadaVertexShader = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec3 VertexColor;

uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform int totalInstances;
uniform float spread;

mat4 rotationMatrix(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main() {
    float id = float(gl_InstanceID);
    float frac = id / float(max(totalInstances - 1, 1));

    // spread the cubes over a sphere
    float phi = acos(1.0 - 2.0 * frac);
    float theta = 3.14159265 * 1.6180339887 * id;

    float r = spread * (0.35 + 0.65 * fract(sin(id * 12.9898) * 43758.5453));
    vec3 basePos = vec3(
        r * sin(phi) * cos(theta),
        r * cos(phi),
        r * sin(phi) * sin(theta)
    );

    // orbit
    float orbitAngle = time * (0.2 + 0.6 * fract(id * 0.137));
    vec3 worldPos = vec3(
        basePos.x * cos(orbitAngle) - basePos.z * sin(orbitAngle),
        basePos.y + sin(time * 2.0 + id) * 0.25,
        basePos.x * sin(orbitAngle) + basePos.z * cos(orbitAngle)
    );

    // spin each cube
    float spinAngle = time * (1.5 + fract(id * 0.381) * 2.5);
    mat4 rot = rotationMatrix(vec3(0.577, 0.577, 0.577), spinAngle);

    float scale = 0.16;
    vec4 localPos = rot * vec4(aPos * scale, 1.0);

    FragPos = worldPos + localPos.xyz;
    Normal = mat3(rot) * aNormal;

    VertexColor = vec3(
        0.80 + 0.20 * sin(id * 0.10),
        0.35 + 0.30 * cos(id * 0.05),
        0.10 + 0.10 * sin(id * 0.20)
    );

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

// lighting shader used by the cube and armada
inline const char* litFragmentShader = R"(#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 VertexColor;

struct PointLight {
    vec3 position;
    vec3 color;
};

#define MAX_LIGHTS 8
uniform PointLight lights[MAX_LIGHTS];
uniform int activeLights;
uniform vec3 viewPos;

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 totalLight = vec3(0.12);
    for (int i = 0; i < activeLights && i < MAX_LIGHTS; ++i) {
        vec3 lightDir = normalize(lights[i].position - FragPos);
        float dist = length(lights[i].position - FragPos);
        float attenuation = 1.0 / (1.0 + 0.05 * dist + 0.015 * dist * dist);

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);

        totalLight += (diff + spec * 0.7) * lights[i].color * attenuation;
    }

    FragColor = vec4(VertexColor * totalLight, 1.0);
}
)";

// print errors if a shader fails
inline void checkShaderErrors(unsigned int id, bool isProgram) {
    int success;
    char log[1024];
    if (isProgram) {
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(id, 1024, NULL, log);
            std::cerr << "Shader program failed to link:\n" << log << std::endl;
        }
    } else {
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(id, 1024, NULL, log);
            std::cerr << "Shader failed to compile:\n" << log << std::endl;
        }
    }
}

inline unsigned int compileShader(GLenum type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    checkShaderErrors(shader, false);
    return shader;
}

// compile + link a vertex and fragment shader
inline unsigned int createProgram(const char* vertexSource, const char* fragmentSource) {
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    checkShaderErrors(program, true);

    // dont need these after linking
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

#endif // SHADERS_H
