#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

struct MultipleLightUniforms {
    GLint u_model;
    GLint u_view;
    GLint u_proj;
};

class Lighting {
public:
    static glm::vec3 lightDirection;
    static glm::vec3 lightColor;
    static glm::vec3 lightPosition;

    static MultipleLightUniforms InitMultipleLightUniforms(unsigned int shaderProgram);
    static void SetLightUniforms(const MultipleLightUniforms& uniforms,
        const glm::vec3& cameraPos,
        const glm::vec3& cameraFront);
};