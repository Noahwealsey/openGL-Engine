#include "Mesh.h"
#include <iostream>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    setupMesh();
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  //bind VBO

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // bind EBO

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void Mesh::Draw(unsigned int &shader) {
    // Bind textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    bool hasDiffuse = false;
    bool hasSpecular = false;

    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);

        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse") {
            number = std::to_string(diffuseNr++);
            hasDiffuse = true;
        }
        else if (name == "texture_specular") {
            number = std::to_string(specularNr++);
            hasSpecular = true;
        }

        // Set the sampler to the correct texture unit
        std::string uniformName = "material." + name + number;
        glUniform1i(glGetUniformLocation(shader, uniformName.c_str()), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    // Create fallback textures for missing materials
    static unsigned int whiteTexture = 0;
    static unsigned int blackTexture = 0;
    static unsigned int grayTexture = 0;

    if (whiteTexture == 0) {
        // White texture for missing diffuse (neutral color)
        glGenTextures(1, &whiteTexture);
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
        unsigned char whitePixel[] = { 200, 200, 200, 255 }; // Light gray instead of pure white
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Black texture for specular (no reflection)
        glGenTextures(1, &blackTexture);
        glBindTexture(GL_TEXTURE_2D, blackTexture);
        unsigned char blackPixel[] = { 0, 0, 0, 155 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blackPixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Gray texture for metallic parts
        glGenTextures(1, &grayTexture);
        glBindTexture(GL_TEXTURE_2D, grayTexture);
        unsigned char grayPixel[] = { 128, 128, 128, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, grayPixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    // Bind fallback textures for missing materials
    if (!hasDiffuse) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grayTexture); // Use gray for untextured parts
        glUniform1i(glGetUniformLocation(shader, "material.texture_diffuse1"), 0);
    }

    if (!hasSpecular) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blackTexture); // No specular reflection
        glUniform1i(glGetUniformLocation(shader, "material.texture_specular1"), 1);
    }

    // Draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Reset to default
    glActiveTexture(GL_TEXTURE0);
}