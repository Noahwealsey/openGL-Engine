#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"
#include "TextureLoader.h"

class Model {
public:
    std::vector<Mesh> meshes;
    std::string directory;

    Model(const std::string& path) {
        loadModel(path);
    }

    void Draw(GLuint& shader) {
        for (auto& mesh : meshes) {
            mesh.Draw(shader);
        }
    }

private:
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);

    // Add these:
    const aiScene* scene; // Store scene for texture loading
    std::vector<Texture> textures_loaded; // Cache loaded textures

    // Add this method declaration:
    unsigned int loadTextureFromMemory(unsigned char* data, int width, int height, int channels);
};