#pragma once
#include <string>
#include <glad/glad.h>

struct ShaderProgrammerSource {
    std::string vertexShaderSource;
    std::string fragmentShaderSource;
};

class Shader {
public:
    static ShaderProgrammerSource ParseShader(const std::string& filePath);
    static unsigned int CompileShader(unsigned int type, const std::string& source);
    static unsigned int CreateShaders(const std::string& vertexShader, const std::string& fragmentShader);

    // OpenGL error handling
    static void ClearError();
    static bool LogCall(const char* function, const char* file, int line);
};