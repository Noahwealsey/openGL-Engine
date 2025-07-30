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

float circlePath = sin(glfwGetTime()) * sin(glfwGetTime()) +
cos(glfwGetTime()) * cos(glfwGetTime());

glm::vec3 lightPos(-.1f, 1.2f, 10.f * circlePath);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f); 
glm::vec3 objectColor(0.8f, 0.4f, 0.6f); 

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


    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
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
        glm::vec3(0.0f, 0.0f, 0.0f), // Center
        glm::vec3(2.0f, 0.0f, 0.0f), // Right
        glm::vec3(-2.0f, 0.0f, 0.0f), // Left
        glm::vec3(0.0f, 2.0f, 0.0f), // Up
        glm::vec3(0.0f, -2.0f, 0.0f) // Down
	};

    float lightVertices[] = {
        // Positions (x, y, z) for a cube scaled to 0.1 (side length 0.2)
        // Back face (facing -Z)
        -0.1f, -0.1f, -0.1f, // Vertex 0
         0.1f, -0.1f, -0.1f, // Vertex 1
         0.1f,  0.1f, -0.1f, // Vertex 2
        -0.1f,  0.1f, -0.1f, // Vertex 3

        // Front face (facing +Z)
        -0.1f, -0.1f,  0.1f, // Vertex 4
         0.1f, -0.1f,  0.1f, // Vertex 5
         0.1f,  0.1f,  0.1f, // Vertex 6
        -0.1f,  0.1f,  0.1f, // Vertex 7

        // Left face (facing -X)
        -0.1f,  0.1f,  0.1f, // Vertex 8
        -0.1f,  0.1f, -0.1f, // Vertex 9
        -0.1f, -0.1f, -0.1f, // Vertex 10
        -0.1f, -0.1f,  0.1f, // Vertex 11

        // Right face (facing +X)
         0.1f,  0.1f,  0.1f, // Vertex 12
         0.1f,  0.1f, -0.1f, // Vertex 13
         0.1f, -0.1f, -0.1f, // Vertex 14
         0.1f, -0.1f,  0.1f, // Vertex 15

         // Bottom face (facing -Y)
         -0.1f, -0.1f, -0.1f, // Vertex 16
          0.1f, -0.1f, -0.1f, // Vertex 17
          0.1f, -0.1f,  0.1f, // Vertex 18
         -0.1f, -0.1f,  0.1f, // Vertex 19

         // Top face (facing +Y)
         -0.1f,  0.1f, -0.1f, // Vertex 20
          0.1f,  0.1f, -0.1f, // Vertex 21
          0.1f,  0.1f,  0.1f, // Vertex 22
         -0.1f,  0.1f,  0.1f  // Vertex 23
    };

    unsigned int VBO, VAO, EBO;
    unsigned int lightVAO, lightVBO, lightEBO;  // Add separate EBO for light

    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glGenBuffers(1, &lightEBO);  // Generate separate EBO

    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);  // Use separate EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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
    ShaderProgrammerSource lightSource= ParseShader("resources/Shaders/bulb_shader.glsl");
    unsigned int shader = createShaders(source.vertexShaderSource, source.fragmentShaderSource);
    unsigned int lightShader = createShaders(lightSource.vertexShaderSource, lightSource.fragmentShaderSource);
    
    unsigned int diffuseMap = loadTexture("resources/Textures/container2.png");
    unsigned int specularMap = loadTexture("resources/Textures/container2_specular.png");


	glEnable(GL_DEPTH_TEST); // Enable depth testing 

	//int location = glGetUniformLocation(shader, "color");
 //   int timeLocation = glGetUniformLocation(shader, "time");
    if (shader == 0) {
        std::cerr << "Failed to create shader program\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
	}
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

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

        

        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Teal
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader);

        glUniformMatrix4fv(glGetUniformLocation(shader, "u_proj"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader, "u_view"), 1, GL_FALSE, glm::value_ptr(view));
        lightPos.x = sin(glfwGetTime()) * 2.0f;
        lightPos.z = cos(glfwGetTime()) * 2.0f;
        glUniform3fv(glGetUniformLocation(shader, "light.position"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(shader, "cameraPos"), 1, glm::value_ptr(cameraPos));

        GLint matAmbientLoc = glGetUniformLocation (shader, "material.ambient");
        glUniform1i(glGetUniformLocation(shader, "material.diffuse"), 0); // GL_TEXTURE0
        glUniform1i(glGetUniformLocation(shader, "material.specular"), 1); // GL_TEXTURE1
        GLint matShineLoc = glGetUniformLocation   (shader, "material.shininess");

        glUniform3f(matAmbientLoc, 0.2f, 0.2f, 0.2f);
        glUniform1f(matShineLoc, 16.0f);

        GLint lightAmbientLoc = glGetUniformLocation (shader, "light.ambient");
        GLint lightDiffuseLoc = glGetUniformLocation (shader, "light.diffuse");
        GLint lightSpecularLoc = glGetUniformLocation(shader, "light.specular");
		GLint lightColorLoc = glGetUniformLocation(shader, "light.color");

        glUniform3f(lightAmbientLoc, 0.2f, 0.2f, 0.2f);
        glUniform3f(lightDiffuseLoc, 1.0f, 1.0f, 1.0f);
        glUniform3f(lightSpecularLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightColorLoc, lightColor.x, lightColor.y, lightColor.z);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        for (int i = 0; i < 5; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePosition[i]); // Translate to the position
            //model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate over time
            glUniformMatrix4fv(glGetUniformLocation(shader, "u_modal"), 1, GL_FALSE, glm::value_ptr(model));
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Your ImGui calls
        ImGui::Begin("Debug");
        ImGui::Text("Greetings, Peasant!");
        ImGui::Begin("Shader Controls");

        ImGui::SliderFloat3("Light Position", glm::value_ptr(lightPos), -10.0f, 10.0f);
        ImGui::ColorEdit3("Light Color", glm::value_ptr(lightColor));

        ImGui::End();

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glUseProgram(lightShader);

        glUniformMatrix4fv(glGetUniformLocation(lightShader, "u_proj"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lightShader, "u_view"), 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, lightPos);
        glUniformMatrix4fv(glGetUniformLocation(lightShader, "u_modal"), 1, GL_FALSE, glm::value_ptr(lightModel));
        glUniform3fv(glGetUniformLocation(lightShader, "lightColor"), 1, glm::value_ptr(lightColor));

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);



        //glUniform1f(glGetUniformLocation(shader, "time"), currentFrame);
        // 
        //glUniform1f(timeLocation, currentFrame);
        //glUniform3f(location, 1.0f, 0.f, 0.f);

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
