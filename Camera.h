#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera {
public:
    // Camera attributes
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;

    // Camera settings
    float yaw;
    float pitch;
    float fov;
    float sensitivity;
    float speed;

    // Mouse tracking
    bool firstMouse;
    float lastX;
    float lastY;

    // Camera mode
    bool cameraMode;

    Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f));

    void ProcessKeyboard(GLFWwindow* window, float deltaTime);
    void ProcessMouseMovement(double xpos, double ypos);
    glm::mat4 GetViewMatrix();

private:
    void updateCameraVectors();
};