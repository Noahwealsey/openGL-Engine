#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

struct ShaderProgramSource {
    std::string vertexShaderSource;
    std::string fragmentShaderSource;
};

class Shader {
public:
    Shader(const std::string& filepath);
    ~Shader();

    void Use() const;
    bool IsValid() const { return m_program != 0; }
    unsigned int GetID() const { return m_program; }

    // Utility uniform functions
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec2(const std::string& name, float x, float y) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec3(const std::string& name, float x, float y, float z) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetVec4(const std::string& name, float x, float y, float z, float w) const;
    void SetMatrix2(const std::string& name, const glm::mat2& mat) const;
    void SetMatrix3(const std::string& name, const glm::mat3& mat) const;
    void SetMatrix4(const std::string& name, const glm::mat4& mat) const;

private:
    unsigned int m_program;

    // Utility functions
    ShaderProgramSource ParseShader(const std::string& filepath);
    unsigned int CompileShader(unsigned int type, const std::string& source);
    unsigned int CreateShaderProgram(const std::string& vertexShader, const std::string& fragmentShader);
};