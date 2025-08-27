#include "Lighting.h"
#include <string>

Lighting::Lighting() {
    // Initialize point light positions
    m_pointLightPositions[0] = glm::vec3(0.7f, 0.2f, 2.0f);
    m_pointLightPositions[1] = glm::vec3(2.3f, -3.3f, -4.0f);
    m_pointLightPositions[2] = glm::vec3(-4.0f, 2.0f, -12.0f);
    m_pointLightPositions[3] = glm::vec3(0.0f, 0.0f, -3.0f);

    // Initialize point light colors
    m_pointLightColors[0] = glm::vec3(1.0f, 0.6f, 0.0f);  // Orange
    m_pointLightColors[1] = glm::vec3(1.0f, 0.0f, 0.0f);  // Red
    m_pointLightColors[2] = glm::vec3(1.0f, 1.0f, 0.0f);  // Yellow
    m_pointLightColors[3] = glm::vec3(0.2f, 0.2f, 1.0f);  // Blue
}

MultipleLightUniforms Lighting::InitializeUniforms(unsigned int shaderProgram) {
    MultipleLightUniforms uniforms;

    uniforms.u_model = glGetUniformLocation(shaderProgram, "u_model");
    uniforms.u_view = glGetUniformLocation(shaderProgram, "u_view");
    uniforms.u_proj = glGetUniformLocation(shaderProgram, "u_proj");

    // Material uniforms
    uniforms.materialDiffuse = glGetUniformLocation(shaderProgram, "material.diffuse");
    uniforms.materialSpecular = glGetUniformLocation(shaderProgram, "material.specular");
    uniforms.materialShininess = glGetUniformLocation(shaderProgram, "material.shininess");

    // View position
    uniforms.viewPos = glGetUniformLocation(shaderProgram, "viewPos");

    // Directional light
    uniforms.dirLightDirection = glGetUniformLocation(shaderProgram, "dirLight.direction");
    uniforms.dirLightAmbient = glGetUniformLocation(shaderProgram, "dirLight.ambient");
    uniforms.dirLightDiffuse = glGetUniformLocation(shaderProgram, "dirLight.diffuse");
    uniforms.dirLightSpecular = glGetUniformLocation(shaderProgram, "dirLight.specular");

    // Point lights (array indexing)
    for (int i = 0; i < 4; i++) {
        std::string number = std::to_string(i);
        uniforms.pointLightPosition[i] = glGetUniformLocation(shaderProgram, ("pointLights[" + number + "].position").c_str());
        uniforms.pointLightConstant[i] = glGetUniformLocation(shaderProgram, ("pointLights[" + number + "].constant").c_str());
        uniforms.pointLightLinear[i] = glGetUniformLocation(shaderProgram, ("pointLights[" + number + "].linear").c_str());
        uniforms.pointLightQuadratic[i] = glGetUniformLocation(shaderProgram, ("pointLights[" + number + "].quadratic").c_str());
        uniforms.pointLightAmbient[i] = glGetUniformLocation(shaderProgram, ("pointLights[" + number + "].ambient").c_str());
        uniforms.pointLightDiffuse[i] = glGetUniformLocation(shaderProgram, ("pointLights[" + number + "].diffuse").c_str());
        uniforms.pointLightSpecular[i] = glGetUniformLocation(shaderProgram, ("pointLights[" + number + "].specular").c_str());
    }

    // Spotlight
    uniforms.spotLightPosition = glGetUniformLocation(shaderProgram, "spotLight.position");
    uniforms.spotLightDirection = glGetUniformLocation(shaderProgram, "spotLight.direction");
    uniforms.spotLightCutOff = glGetUniformLocation(shaderProgram, "spotLight.cutOff");
    uniforms.spotLightOuterCutOff = glGetUniformLocation(shaderProgram, "spotLight.outerCutOff");
    uniforms.spotLightConstant = glGetUniformLocation(shaderProgram, "spotLight.constant");
    uniforms.spotLightLinear = glGetUniformLocation(shaderProgram, "spotLight.linear");
    uniforms.spotLightQuadratic = glGetUniformLocation(shaderProgram, "spotLight.quadratic");
    uniforms.spotLightAmbient = glGetUniformLocation(shaderProgram, "spotLight.ambient");
    uniforms.spotLightDiffuse = glGetUniformLocation(shaderProgram, "spotLight.diffuse");
    uniforms.spotLightSpecular = glGetUniformLocation(shaderProgram, "spotLight.specular");

    return uniforms;
}

void Lighting::SetLightUniforms(const MultipleLightUniforms& uniforms,
                               const glm::vec3& cameraPos,
                               const glm::vec3& cameraFront) {
    // Material properties
    glUniform1i(uniforms.materialDiffuse, 0);
    glUniform1i(uniforms.materialSpecular, 1);
    glUniform1f(uniforms.materialShininess, 32.0f);

    // View position
    glUniform3fv(uniforms.viewPos, 1, glm::value_ptr(cameraPos));

    // Directional light (sun-like light from above)
    glUniform3f(uniforms.dirLightDirection, -0.2f, -1.0f, -0.3f);
    glUniform3f(uniforms.dirLightAmbient, 0.05f, 0.05f, 0.05f);
    glUniform3f(uniforms.dirLightDiffuse, 0.4f, 0.4f, 0.4f);
    glUniform3f(uniforms.dirLightSpecular, 0.5f, 0.5f, 0.5f);

    // Point lights
    for (int i = 0; i < 4; i++) {
        glUniform3fv(uniforms.pointLightPosition[i], 1, glm::value_ptr(m_pointLightPositions[i]));
        glUniform1f(uniforms.pointLightConstant[i], 1.0f);
        glUniform1f(uniforms.pointLightLinear[i], 0.09f);
        glUniform1f(uniforms.pointLightQuadratic[i], 0.032f);

        // Use different colors for variety
        glm::vec3 color = m_pointLightColors[i];
        glUniform3f(uniforms.pointLightAmbient[i], color.x * 0.1f, color.y * 0.1f, color.z * 0.1f);
        glUniform3f(uniforms.pointLightDiffuse[i], color.x, color.y, color.z);
        glUniform3f(uniforms.pointLightSpecular[i], 1.0f, 1.0f, 1.0f);
    }

    // Spotlight (flashlight attached to camera)
    glUniform3fv(uniforms.spotLightPosition, 1, glm::value_ptr(cameraPos));
    glUniform3fv(uniforms.spotLightDirection, 1, glm::value_ptr(cameraFront));
    glUniform1f(uniforms.spotLightCutOff, glm::cos(glm::radians(12.5f)));
    glUniform1f(uniforms.spotLightOuterCutOff, glm::cos(glm::radians(15.0f)));
    glUniform1f(uniforms.spotLightConstant, 1.0f);
    glUniform1f(uniforms.spotLightLinear, 0.09f);
    glUniform1f(uniforms.spotLightQuadratic, 0.032f);
    glUniform3f(uniforms.spotLightAmbient, 0.0f, 0.0f, 0.0f);
    glUniform3f(uniforms.spotLightDiffuse, 1.0f, 1.0f, 1.0f);
    glUniform3f(uniforms.spotLightSpecular, 1.0f, 1.0f, 1.0f);
}

void Lighting::UpdateSpotlightCutoff(unsigned int shaderProgram, float innerCutoff, float outerCutoff) {
    glUseProgram(shaderProgram);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.cutOff"), glm::cos(glm::radians(innerCutoff)));
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.outerCutOff"), glm::cos(glm::radians(outerCutoff)));
}