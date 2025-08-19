#include "Lights.h"

// Initialize static variables
glm::vec3 Lighting::lightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
glm::vec3 Lighting::lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 Lighting::lightPosition = glm::vec3(1.0f, 1.0f, 1.0f);

MultipleLightUniforms Lighting::InitMultipleLightUniforms(unsigned int shaderProgram) {
    MultipleLightUniforms uniforms;

    uniforms.u_model = glGetUniformLocation(shaderProgram, "u_model");
    uniforms.u_view = glGetUniformLocation(shaderProgram, "u_view");
    uniforms.u_proj = glGetUniformLocation(shaderProgram, "u_proj");

    return uniforms;
}

void Lighting::SetLightUniforms(const MultipleLightUniforms& uniforms,
    const glm::vec3& cameraPos,
    const glm::vec3& cameraFront) {
    // Implementation of your lighting uniform setting logic
    // This was empty in your original code, so you can fill it in based on your needs
}