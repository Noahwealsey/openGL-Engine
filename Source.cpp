#include "Renderer.h"
#include "Camera.h"
#include "Shader.h"
#include "Texture.h"
#include "Lighting.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    if (!Renderer::Initialize()) {
        return -1;
    }

    GLFWwindow* window = Renderer::GetWindow();
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
    
    // Set up camera callbacks
    glfwSetWindowUserPointer(window, &camera);
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        cam->ProcessMouseMovement(xpos, ypos);
    });

    // Load shaders
    Shader lightingShader("resources/Shaders/Basic_shader.glsl");
    Shader lightCubeShader("resources/Shaders/bulb_shader.glsl");

    if (!lightingShader.IsValid() || !lightCubeShader.IsValid()) {
        std::cerr << "Failed to load shaders!" << std::endl;
        return -1;
    }

    // Load textures
    Texture diffuseMap("resources/Textures/container2.png");
    Texture specularMap("resources/Textures/container2_specular.png");

    // Initialize lighting system
    Lighting lighting;
    MultipleLightUniforms uniforms = lighting.InitializeUniforms(lightingShader.GetID());

    // Set up cube data
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
        0, 1, 2,   2, 3, 0,
        // Front face (vertices 4-7)
        4, 5, 6,   6, 7, 4,
        // Left face (vertices 8-11)
        8, 9, 10,  10, 11, 8,
        // Right face (vertices 12-15)
        12, 13, 14, 14, 15, 12,
        // Bottom face (vertices 16-19)
        16, 17, 18, 18, 19, 16,
        // Top face (vertices 20-23)
        20, 21, 22, 22, 23, 20
    };

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),   glm::vec3(2.0f, 0.0f, 0.0f),   glm::vec3(-2.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 2.0f, 0.0f),   glm::vec3(0.0f, -2.0f, 0.0f),  glm::vec3(0.0f, 0.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, -2.0f),  glm::vec3(2.0f, 2.0f, 0.0f),   glm::vec3(2.0f, -2.0f, 0.0f),
        glm::vec3(-2.0f, 2.0f, 0.0f),  glm::vec3(-2.0f, -2.0f, 0.0f), glm::vec3(2.0f, 0.0f, 2.0f),
        glm::vec3(2.0f, 0.0f, -2.0f),  glm::vec3(-2.0f, 0.0f, 2.0f),  glm::vec3(-2.0f, 0.0f, -2.0f),
        glm::vec3(0.0f, 2.0f, 2.0f),   glm::vec3(0.0f, 2.0f, -2.0f),  glm::vec3(0.0f, -2.0f, 2.0f),
        glm::vec3(0.0f, -2.0f, -2.0f), glm::vec3(4.0f, 0.0f, 0.0f),   glm::vec3(-4.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 4.0f, 0.0f),   glm::vec3(0.0f, -4.0f, 0.0f),  glm::vec3(0.0f, 0.0f, 4.0f),
        glm::vec3(0.0f, 0.0f, -4.0f),  glm::vec3(2.0f, 2.0f, 2.0f),   glm::vec3(2.0f, 2.0f, -2.0f),
        glm::vec3(-2.0f, 2.0f, 2.0f),  glm::vec3(-2.0f, 2.0f, -2.0f), glm::vec3(2.0f, -2.0f, 2.0f)
    };

    // Set up vertex arrays and buffers
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Setup ImGui
    Renderer::InitializeImGui();

    // Performance monitoring
    GLuint timerQuery;
    glGenQueries(1, &timerQuery);
    GLuint64 elapsed_time;

    float lastFrame = 0.0f;
    
    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        camera.ProcessKeyboard(window, deltaTime);

        // Start GPU timer
        glBeginQuery(GL_TIME_ELAPSED, timerQuery);

        // Clear
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get view and projection matrices
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        // ==========================================
        // RENDER TEXTURED CUBES WITH LIGHTING
        // ==========================================
        lightingShader.Use();
        glBindVertexArray(VAO);

        // Set matrices
        lightingShader.SetMatrix4("u_view", view);
        lightingShader.SetMatrix4("u_proj", projection);

        // Set lighting uniforms
        lighting.SetLightUniforms(uniforms, camera.GetPosition(), camera.GetFront());

        // Bind textures
        diffuseMap.Bind(0);
        specularMap.Bind(1);

        // Render all textured cubes
        for (int i = 0; i < 30; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, 0.5f * (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
            lightingShader.SetMatrix4("u_model", model);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        // ==========================================
        // RENDER LIGHT CUBES
        // ==========================================
        lightCubeShader.Use();
        lightCubeShader.SetMatrix4("u_view", view);
        lightCubeShader.SetMatrix4("u_proj", projection);

        // Render point lights as colored cubes
        const auto& lightPositions = lighting.GetPointLightPositions();
        const auto& lightColors = lighting.GetPointLightColors();
        
        for (int i = 0; i < 4; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            
            lightCubeShader.SetMatrix4("u_model", model);
            lightCubeShader.SetVec3("lightColor", lightColors[i]);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        // ==========================================
        // IMGUI
        // ==========================================
        Renderer::BeginImGuiFrame();
        
        ImGui::Begin("Shader Controls");
        
        // Light controls
        static glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
        ImGui::ColorEdit3("Light Color", glm::value_ptr(lightColor));

        // Spotlight controls
        static float cutoffAngle = 12.5f;
        static float outerCutoffAngle = 15.0f;
        if (ImGui::SliderFloat("Spotlight Inner Cutoff", &cutoffAngle, 5.0f, 25.0f)) {
            lighting.UpdateSpotlightCutoff(lightingShader.GetID(), cutoffAngle, outerCutoffAngle);
        }
        if (ImGui::SliderFloat("Spotlight Outer Cutoff", &outerCutoffAngle, cutoffAngle + 1.0f, 30.0f)) {
            lighting.UpdateSpotlightCutoff(lightingShader.GetID(), cutoffAngle, outerCutoffAngle);
        }

        ImGui::End();

        Renderer::EndImGuiFrame();

        // End GPU timer
        glEndQuery(GL_TIME_ELAPSED);
        glGetQueryObjectui64v(timerQuery, GL_QUERY_RESULT, &elapsed_time);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteQueries(1, &timerQuery);
    
    Renderer::Cleanup();
    return 0;
}