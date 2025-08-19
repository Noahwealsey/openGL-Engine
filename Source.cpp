#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <cmath>
#include <thread>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Include our custom headers
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "TextureLoader.h"
#include "Lights.h"
#include "Model.h"

// Debug macros
#define BREAKER(x) if(!x) __debugbreak();
#define BREAKCALL(x) Shader::ClearError();\
    x;\
    BREAKER(Shader::LogCall(#x, __FILE__, __LINE__));

// Global camera instance
Camera camera;

// Mouse callback wrapper
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    camera.ProcessMouseMovement(xpos, ypos);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "OpenGL Window", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Shader parsing and loading
    ShaderProgrammerSource source = Shader::ParseShader("resources/Shaders/Basic_shader.glsl");
    unsigned int shader = Shader::CreateShaders(source.vertexShaderSource, source.fragmentShaderSource);

    MultipleLightUniforms uniforms = Lighting::InitMultipleLightUniforms(shader);

    glEnable(GL_DEPTH_TEST); // Enable depth testing 

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

    float lastFrame = 0.0f;

	Model myModel("resources/Models/supra.glb");

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        camera.ProcessKeyboard(window, deltaTime);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            (float)800 / (float)600,
            0.1f, 100.0f);

        glBeginQuery(GL_TIME_ELAPSED, timerQuery);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);

        // Set matrices for lighting shader
        glUniformMatrix4fv(uniforms.u_proj, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniforms.u_view, 1, GL_FALSE, glm::value_ptr(view));

        // Set all lighting uniforms
        Lighting::SetLightUniforms(uniforms, camera.position, camera.front);

        glUniform1i(glGetUniformLocation(shader, "material.texture_diffuse1"), 0);
        glUniform1i(glGetUniformLocation(shader, "material.texture_specular1"), 1);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Position model
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f)); // Scale model
        //model = glm::rotate(model, 0.5f * (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate over time
        glUniformMatrix4fv(glGetUniformLocation(shader, "u_model"), 1, GL_FALSE, glm::value_ptr(model));
        myModel.Draw(shader);

        // ==========================================
        // IMGUI
        // ==========================================
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Shader Controls");

        // Update spotlight uniforms in real-time
        glUseProgram(shader);

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
    return 0;
}