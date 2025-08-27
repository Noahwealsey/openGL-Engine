#pragma once

#include <glad/glad.h>
#include <string>

class Texture {
public:
    Texture(const std::string& path);
    ~Texture();

    void Bind(unsigned int slot = 0) const;
    void Unbind() const;
    
    unsigned int GetID() const { return m_textureID; }
    bool IsValid() const { return m_textureID != 0; }
    
    // Getters for texture properties
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetChannels() const { return m_channels; }

private:
    unsigned int m_textureID;
    int m_width, m_height, m_channels;
    std::string m_filePath;
};