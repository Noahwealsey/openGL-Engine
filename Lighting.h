#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct MultipleLightUniforms {
    GLint u_model;
    GLint u_view;
    GLint u_proj;

    // Material
    GLint materialDiffuse;
    GLint materialSpecular;
    GLint materialShininess;

    // View position
    GLint viewPos;

    // Directional light
    GLint dirLightDirection;
    GLint dirLightAmbient;
    GLint dirLightDiffuse;
    GLint dirLightSpecular;

    // Point lights array
    GLint pointLightPosition[4];
    GLint pointLightConstant[4];
    GLint pointLightLinear[4];
    GLint pointLightQuadratic[4];
    GLint pointLightAmbient[4];
    GLint pointLightDiffuse[4];
    GLint pointLightSpecular[4];

    // Spotlight
    GLint spotLightPosition;
    GLint spotLightDirection;
    GLint spotLightCutOff;
    GLint spotLightOuterCutOff;
    GLint spotLightConstant;
    GLint spotLightLinear;
    GLint spotLightQuadratic;
    GLint spotLightAmbient;
    GLint spotLightDiffuse;
    GLint spotLightSpecular;
};

class Lighting {
public:
    Lighting();
    
    MultipleLightUniforms InitializeUniforms(unsigned int shaderProgram);
    void SetLightUniforms(const MultipleLightUniforms& uniforms,
                         const glm::vec3& cameraPos,
                         const glm::vec3& cameraFront);
    
    void UpdateSpotlightCutoff(unsigned int shaderProgram, float innerCutoff, float outerCutoff);
    
    // Getters
    const glm::vec3* GetPointLightPositions() const { return m_pointLightPositions; }
    const glm::vec3* GetPointLightColors() const { return m_pointLightColors; }

private:
    glm::vec3 m_pointLightPositions[4];
    glm::vec3 m_pointLightColors[4];
};