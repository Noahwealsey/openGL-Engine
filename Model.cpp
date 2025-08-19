#include "Model.h"

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    // Store scene for texture loading
    this->scene = scene;
    directory = path.substr(0, path.find_last_of('/')); // Extract directory for texture paths
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    // Process all meshes in this node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Extract vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        if (mesh->mTextureCoords[0]) {
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }

    // Extract indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Extract textures
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string texturePath = str.C_Str();

        // Check if texture was already loaded
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.data(), texturePath.c_str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }

        if (!skip) {
            Texture texture;

            // Check if texture is embedded (starts with '*')
            if (texturePath[0] == '*') {
                unsigned int index = std::stoi(texturePath.substr(1)); // Extract index after '*'
                if (index < scene->mNumTextures) {
                    aiTexture* aiTex = scene->mTextures[index];
                    if (aiTex->mHeight == 0) { // Compressed texture
                        unsigned char* data = reinterpret_cast<unsigned char*>(aiTex->pcData);
                        int size = aiTex->mWidth;
                        // Load texture manually using stb_image
                        int width, height, channels;
                        unsigned char* image = stbi_load_from_memory(data, size, &width, &height, &channels, 0);
                        if (image) {
                            texture.id = loadTextureFromMemory(image, width, height, channels);
                            texture.type = typeName;
                            texture.path = texturePath;
                            stbi_image_free(image);
                        }
                        else {
                            std::cout << "Failed to load embedded texture: " << texturePath << std::endl;
                            continue;
                        }
                    }
                    else {
                        // Uncompressed embedded texture
                        texture.id = loadTextureFromMemory(reinterpret_cast<unsigned char*>(aiTex->pcData),
                            aiTex->mWidth, aiTex->mHeight, 4);
                        texture.type = typeName;
                        texture.path = texturePath;
                    }
                }
                else {
                    std::cout << "Invalid embedded texture index: " << index << std::endl;
                    continue;
                }
            }
            else {
                // External texture file
                std::string fullPath = directory + '/' + texturePath;
                texture.id = TextureLoader::LoadTexture(fullPath.c_str());
                texture.type = typeName;
                texture.path = texturePath;

                if (texture.id == 0) {
                    std::cout << "Failed to load texture: " << fullPath << std::endl;
                    continue;
                }
            }

            textures.push_back(texture);
            textures_loaded.push_back(texture); // Cache loaded texture
        }
    }
    return textures;
}

unsigned int Model::loadTextureFromMemory(unsigned char* data, int width, int height, int channels) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;
    else {
        std::cout << "Unsupported texture format with " << channels << " channels" << std::endl;
        glDeleteTextures(1, &textureID);
        return 0;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}