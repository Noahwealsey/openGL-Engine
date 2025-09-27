#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    // Constructor
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
           float yaw = -90.0f, float pitch = 0.0f);

    // Returns the view matrix calculated using Euler angles and LookAt matrix
    glm::mat4 GetViewMatrix() const;

    // Process input received from keyboard
    void ProcessKeyboard(GLFWwindow* window, float deltaTime);
    
    // Process input received from mouse movement
    void ProcessMouseMovement(double xpos, double ypos, bool constrainPitch = true);

    // Process input received from mouse scroll wheel
    void ProcessMouseScroll(float yoffset);

    // Getters
    glm::vec3 GetPosition() const { return m_position; }
    glm::vec3 GetFront() const { return m_front; }
    glm::vec3 GetUp() const { return m_up; }
    glm::vec3 GetRight() const { return m_right; }
    float GetZoom() const { return m_zoom; }
    bool IsCameraMode() const { return m_cameraMode; }

private:
    // Camera attributes
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;

    float gravity = 9.81f;
    float groundLevel = 1.0f;
    float verticalSpeed = 0.0f;
    float onGround = true;

    // Euler angles
    float m_yaw;
    float m_pitch;

    // Camera options
    float m_movementSpeed;
    float m_mouseSensitivity;
    float m_zoom;

    // Mouse input
    bool m_firstMouse;
    float m_lastX;
    float m_lastY;

    // Camera mode toggle
    bool m_cameraMode;

    // Calculates the front vector from the Camera's (updated) Euler angles
    void UpdateCameraVectors();
};