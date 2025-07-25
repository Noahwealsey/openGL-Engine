#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define BREAKER(x) if(!x) __debugbreak();
#define BREAKCALL(x) ClearError();\
    x;\
    BREAKER(LogCall(#x, __FILE__, __LINE__));

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 lightPos(-1.2f, 1.2f, 2.f); // Move light further away and higher
glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // Pure white light
glm::vec3 objectColor(0.8f, 0.4f, 0.6f); // Lighter color to see effects better

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
float yaw = -90.0f; // looking towards -Z
float pitch = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
float fov = 45.0f;

float lastFrame = 0.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
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

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        std::cout << "Light pos: " << lightPos.x << ", " << lightPos.y << ", " << lightPos.z << std::endl;
        std::cout << "Camera pos: " << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << std::endl;
    }

}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	

    float vertices[] = {
        // Positions         // Normals
        // Back face (the one you see first, facing -Z)
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        // Front face (facing +Z)
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        // Left face (facing -X)
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

        // Right face (facing +X)
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

         // Bottom face (facing -Y)
         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
          0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,

         // Top face (facing +Y)
         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
          0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
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


    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // normal
    glEnableVertexAttribArray(1);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);


    //shader parsing and loading
	ShaderProgrammerSource source = ParseShader("resources/Shaders/Basic_shader.glsl");
	unsigned int shader = createShaders(source.vertexShaderSource, source.fragmentShaderSource);
    
	glEnable(GL_DEPTH_TEST); // Enable depth testing 

	//int location = glGetUniformLocation(shader, "color");
 //   int timeLocation = glGetUniformLocation(shader, "time");
    if (shader == 0) {
        std::cerr << "Failed to create shader program\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
	}

    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window, deltaTime);
        glm::mat4 model = glm::mat4(1.0f);
        //model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate over time

        glm::mat4 view = glm::lookAt(
            cameraPos,              // Position of the camera
            cameraPos + cameraFront,// Look direction
            cameraUp                // Up vector
        );

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            (float)800 / (float)600,
            0.1f, 100.0f);

        //glm::mat4 mvp = projection * view * model; // Model-View-Projection 

        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Teal
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);

        glUniformMatrix4fv(glGetUniformLocation(shader, "u_proj"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader, "u_view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader, "u_modal"), 1, GL_FALSE, glm::value_ptr(model));

        glUniform3fv(glGetUniformLocation(shader, "lightPos"), 1, glm::value_ptr(lightPos));
		glUniform3fv(glGetUniformLocation(shader, "lightColor"), 1, glm::value_ptr(lightColor));
		glUniform3fv(glGetUniformLocation(shader, "objectColor"), 1, glm::value_ptr(objectColor));
        glUniform3fv(glGetUniformLocation(shader, "cameraPos"), 1, glm::value_ptr(cameraPos));
		
    
        //glUniform1f(glGetUniformLocation(shader, "time"), currentFrame);
        // 
        //glUniform1f(timeLocation, currentFrame);
        //glUniform3f(location, 1.0f, 0.f, 0.f);

        // Draw using EBO (no need to call glDrawArrays here)
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

	glDeleteShader(shader);
    glfwDestroyWindow(window);
    glfwTerminate();
}
