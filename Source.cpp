#include<iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cmath>
#include <thread>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define BREAKER(x) if(!x) __debugbreak();
#define BREAKCALL(x) ClearError();\
    x;\
    BREAKER(LogCall(#x, __FILE__, __LINE__));

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 lightDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f); 
glm::vec3 lightPosition(1.0f, 1.0f, 1.0f); 

static void ClearError() {
    while (glGetError() != GL_NO_ERROR);
}

static bool LogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::cerr << "[OpenGL Error] (" << error << "): " << function
                  << " in " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

struct ShaderProgrammerSource {
    std::string vertexShaderSource;
    std::string fragmentShaderSource;
};

ShaderProgrammerSource ParseShader(const std::string& filePath) {
    std::ifstream stream(filePath);
    
    enum class ShaderType{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
    };
	
	ShaderType type = ShaderType::NONE; 
    std::stringstream ss[2];
    std::string line;
    while (getline(stream, line)) {
        if(line.find("#shader") != std::string::npos){
			if(line.find("Vertex") != std::string::npos){
				type = ShaderType::VERTEX;
			}
			else if(line.find("Fragment") != std::string::npos){
				type = ShaderType::FRAGMENT;
			}
		}
		else {
			ss[(int)type] << line << "\n";
		}
    }

	return {ss[0].str(), ss[1].str()};
    
}


static unsigned int compileShaders(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    // Error handling
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = new char[length];
        glGetShaderInfoLog(id, length, &length, message);
        std::cerr << "Failed to compile shader!\n" << message << std::endl;
        delete[] message;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

static unsigned int createShaders(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShaders(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShaders(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;

}

//Camera control variables
bool firstMouse = true;
float yaw = -90.0f; 
float pitch = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
float fov = 45.0f;
bool cameraMode = true; //for togglinr camera control and mouse cursor visibility
float lastFrame = 0.0f;

glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f,  0.2f,  2.0f),
    glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3(0.0f,  0.0f, -3.0f)
};


glm::vec3 pointLightColors[] = {
    glm::vec3(1.0f, 0.6f, 0.0f),  // Orange
    glm::vec3(1.0f, 0.0f, 0.0f),  // Red
    glm::vec3(1.0f, 1.0f, 0.0f),  // Yellow
    glm::vec3(0.2f, 0.2f, 1.0f)   // Blue
};


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


MultipleLightUniforms initMultipleLightUniforms(unsigned int shaderProgram) {
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

void setLightUniforms(const MultipleLightUniforms& uniforms,
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
        glUniform3fv(uniforms.pointLightPosition[i], 1, glm::value_ptr(pointLightPositions[i]));
        glUniform1f(uniforms.pointLightConstant[i], 1.0f);
        glUniform1f(uniforms.pointLightLinear[i], 0.09f);
        glUniform1f(uniforms.pointLightQuadratic[i], 0.032f);

        // Use different colors for variety
        glm::vec3 color = pointLightColors[i];
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

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!cameraMode) {
        return;
    }
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

	// important to prevent Gimbal lock 
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void processInput(GLFWwindow* window, float delta) {
    float cameraSpeed = 2.5f *delta;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		//std::cout << "W pressed" << std::endl;
        cameraPos += cameraSpeed * cameraFront;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) 
		cameraPos.y += cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        cameraMode = !cameraMode;
        if (cameraMode) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        // Simple debounce: wait a bit to avoid rapid toggling
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true); // Flip image
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);

    if (data) {
        GLenum format = (nrComponents == 3) ? GL_RGB : GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(1920, 1080    , "OpenGL Window", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	

    float vertices[] = {
        // Positions          // Normals            // TexCoords
        // Back face (-Z)
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,

        // Front face (+Z)
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,

        // Left face (-X)
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

        // Right face (+X)
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,

         // Bottom face (-Y)
         -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,

         // Top face (+Y)
         -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
          0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f
    };


    unsigned int indices[] = {
        // Back face (vertices 0-3)
        0, 1, 2,
        2, 3, 0,
        // Front face (vertices 4-7)
        4, 5, 6,
        6, 7, 4,
        // Left face (vertices 8-11)
        8, 9, 10,
        10, 11, 8,
        // Right face (vertices 12-15)
        12, 13, 14,
        14, 15, 12,
        // Bottom face (vertices 16-19)
        16, 17, 18,
        18, 19, 16,
        // Top face (vertices 20-23)
        20, 21, 22,
        22, 23, 20
    };

    glm::vec3 cubePosition[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),   // Center
        glm::vec3(2.0f, 0.0f, 0.0f),   // Right
        glm::vec3(-2.0f, 0.0f, 0.0f),  // Left
        glm::vec3(0.0f, 2.0f, 0.0f),   // Up
        glm::vec3(0.0f, -2.0f, 0.0f),  // Down
        glm::vec3(0.0f, 0.0f, 2.0f),   // Front
        glm::vec3(0.0f, 0.0f, -2.0f),  // Back
        glm::vec3(2.0f, 2.0f, 0.0f),   // Right-Up
        glm::vec3(2.0f, -2.0f, 0.0f),  // Right-Down
        glm::vec3(-2.0f, 2.0f, 0.0f),  // Left-Up
        glm::vec3(-2.0f, -2.0f, 0.0f), // Left-Down
        glm::vec3(2.0f, 0.0f, 2.0f),   // Right-Front
        glm::vec3(2.0f, 0.0f, -2.0f),  // Right-Back
        glm::vec3(-2.0f, 0.0f, 2.0f),  // Left-Front
        glm::vec3(-2.0f, 0.0f, -2.0f), // Left-Back
        glm::vec3(0.0f, 2.0f, 2.0f),   // Up-Front
        glm::vec3(0.0f, 2.0f, -2.0f),  // Up-Back
        glm::vec3(0.0f, -2.0f, 2.0f),  // Down-Front
        glm::vec3(0.0f, -2.0f, -2.0f), // Down-Back
        glm::vec3(4.0f, 0.0f, 0.0f),   // Far-Right
        glm::vec3(-4.0f, 0.0f, 0.0f),  // Far-Left
        glm::vec3(0.0f, 4.0f, 0.0f),   // Far-Up
        glm::vec3(0.0f, -4.0f, 0.0f),  // Far-Down
        glm::vec3(0.0f, 0.0f, 4.0f),   // Far-Front
        glm::vec3(0.0f, 0.0f, -4.0f),  // Far-Back
        glm::vec3(2.0f, 2.0f, 2.0f),   // Right-Up-Front
        glm::vec3(2.0f, 2.0f, -2.0f),  // Right-Up-Back
        glm::vec3(-2.0f, 2.0f, 2.0f),  // Left-Up-Front
        glm::vec3(-2.0f, 2.0f, -2.0f), // Left-Up-Back
        glm::vec3(2.0f, -2.0f, 2.0f)   // Right-Down-Front
    };

    unsigned int VBO, VAO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // texture coords
    glEnableVertexAttribArray(2);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    //shader parsing and loading
    ShaderProgrammerSource source = ParseShader("resources/Shaders/Basic_shader.glsl");
    unsigned int shader = createShaders(source.vertexShaderSource, source.fragmentShaderSource);

    ShaderProgrammerSource lightSourceSource = ParseShader("resources/Shaders/bulb_shader.glsl");
    unsigned int lightShader = createShaders(lightSourceSource.vertexShaderSource, lightSourceSource.fragmentShaderSource);

    MultipleLightUniforms uniforms = initMultipleLightUniforms(shader); // Cache all locations
    unsigned int diffuseMap = loadTexture("resources/Textures/container2.png");
    unsigned int specularMap = loadTexture("resources/Textures/container2_specular.png");

    glEnable(GL_DEPTH_TEST); // Enable depth testing 

    GLint lightCubeModelLoc = glGetUniformLocation(lightShader, "u_model");
    GLint lightCubeViewLoc = glGetUniformLocation(lightShader, "u_view");
    GLint lightCubeProjLoc = glGetUniformLocation(lightShader, "u_proj");
    GLint lightCubeColorLoc = glGetUniformLocation(lightShader, "lightColor");

    if (shader == 0) {
        std::cerr << "Failed to create shader program\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    GLuint timerQuery;
    glGenQueries(1, &timerQuery);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    GLuint64 elapsed_time;

    // Add this BEFORE your render loop to check if shaders compiled
    if (shader == 0) {
        std::cerr << "Main lighting shader failed to compile!" << std::endl;
        return -1;
    }
    if (lightShader == 0) {
        std::cerr << "Light cube shader failed to compile!" << std::endl;
        return -1;
    }

    // Add this to check if textures loaded
    std::cout << "Diffuse texture ID: " << diffuseMap << std::endl;
    std::cout << "Specular texture ID: " << specularMap << std::endl;

    // Check if uniform locations are valid
    std::cout << "Material diffuse location: " << uniforms.materialDiffuse << std::endl;
    std::cout << "Light cube model location: " << lightCubeModelLoc << std::endl;

    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window, deltaTime);

        glm::mat4 view = glm::lookAt(
            cameraPos,              // Position of the camera
            cameraPos + cameraFront,// Look direction
            cameraUp                // Up vector
        );

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            (float)800 / (float)600,
            0.1f, 100.0f);

        glBeginQuery(GL_TIME_ELAPSED, timerQuery);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ==========================================
        // RENDER TEXTURED CUBES WITH LIGHTING
        // ==========================================
        glUseProgram(shader);

        GLint diffuseLoc = glGetUniformLocation(shader, "material.diffuse");
        GLint specularLoc = glGetUniformLocation(shader, "material.specular");
        GLint texDiffuseLoc = glGetUniformLocation(shader, "texture_diffuse1"); // Common alternative name
        GLint texSpecLoc = glGetUniformLocation(shader, "texture_specular1");   // Common alternative name

        std::cout << "material.diffuse location: " << diffuseLoc << std::endl;
        std::cout << "material.specular location: " << specularLoc << std::endl;
        std::cout << "texture_diffuse1 location: " << texDiffuseLoc << std::endl;
        std::cout << "texture_specular1 location: " << texSpecLoc << std::endl;

        glBindVertexArray(VAO);

        // Set matrices for lighting shader
        glUniformMatrix4fv(uniforms.u_proj, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniforms.u_view, 1, GL_FALSE, glm::value_ptr(view));

        // Set all lighting uniforms
        setLightUniforms(uniforms, cameraPos, cameraFront);

        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        // Render all textured cubes
        for (int i = 0; i < 30; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePosition[i]); // Translate to the position
            model = glm::rotate(model, 0.5f*(float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate over time
            glUniformMatrix4fv(glGetUniformLocation(shader, "u_model"), 1, GL_FALSE, glm::value_ptr(model));
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        // ==========================================
        // RENDER LIGHT CUBES (SOLID COLORS)
        // ==========================================
        glUseProgram(lightShader);
        glBindVertexArray(VAO); // Same VAO, different shader

        // Set matrices for light cube shader
        glUniformMatrix4fv(lightCubeProjLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(lightCubeViewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Render each point light as a small colored cube
        for (int i = 0; i < 4; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make light cubes smaller

            glUniformMatrix4fv(lightCubeModelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(lightCubeColorLoc, 1, glm::value_ptr(pointLightColors[i]));

            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0); // Same EBO
        }

        // ==========================================
        // IMGUI
        // ==========================================
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Shader Controls");
        ImGui::ColorEdit3("Light Color", glm::value_ptr(lightColor));

        // Add spotlight controls
        static float cutoffAngle = 12.5f;
        static float outerCutoffAngle = 15.0f;
        ImGui::SliderFloat("Spotlight Inner Cutoff", &cutoffAngle, 5.0f, 25.0f);
        ImGui::SliderFloat("Spotlight Outer Cutoff", &outerCutoffAngle, cutoffAngle + 1.0f, 30.0f);

        // Update spotlight uniforms in real-time
        glUseProgram(shader);
        glUniform1f(glGetUniformLocation(shader, "spotLight.cutOff"), glm::cos(glm::radians(cutoffAngle)));
        glUniform1f(glGetUniformLocation(shader, "spotLight.outerCutOff"), glm::cos(glm::radians(outerCutoffAngle)));

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glEndQuery(GL_TIME_ELAPSED);
        glGetQueryObjectui64v(timerQuery, GL_QUERY_RESULT, &elapsed_time);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }   
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	glDeleteShader(shader);
    glfwDestroyWindow(window);
    glfwTerminate();
}
