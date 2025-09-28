#pragma once
#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

class Model {
public:
    Model(const std::string& path, const std::string& baseDir = "");
    void Draw(); // later: pass shader
    glm::vec3 materialDiffuse = glm::vec3(0.8f); // fallback gray
    glm::vec3 materialSpecular = glm::vec3(0.5f);
    float materialShininess = 32.0f;

private:
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    void setupMesh();
    void loadModel(const std::string& path, const std::string& baseDir);
};
