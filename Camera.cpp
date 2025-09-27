#include "Camera.h"
#include <thread>
#include <chrono>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_position(position)
    , m_worldUp(up)
    , m_yaw(yaw)
    , m_pitch(pitch)
    , m_movementSpeed(2.5f)
    , m_mouseSensitivity(0.1f)
    , m_zoom(45.0f)
    , m_firstMouse(true)
    , m_lastX(400.0f)
    , m_lastY(300.0f)
    , m_cameraMode(true)
{
    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

void Camera::ProcessKeyboard(GLFWwindow* window, float deltaTime) {

    float speedMultiplier = 1.0f;


    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        speedMultiplier = 2.0f;
    }
    float velocity = m_movementSpeed * speedMultiplier*deltaTime;

	verticalSpeed -= gravity * deltaTime;
    

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && onGround) {
		verticalSpeed = 5.0f; // Jump impulse
        onGround = false;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_position += m_front * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_position -= m_front * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_position -= m_right * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_position += m_right * velocity;
	m_position.y += verticalSpeed * deltaTime;
    if (!onGround) {
        verticalSpeed -= gravity * deltaTime;
    }
    if (m_position.y <= groundLevel) {
        m_position.y = groundLevel; // Prevent falling below ground
		verticalSpeed = 0.0f;
		onGround = true;
    }
    // Toggle camera mode with TAB
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        m_cameraMode = !m_cameraMode;
        if (m_cameraMode) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        // Simple debounce: wait a bit to avoid rapid toggling
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void Camera::ProcessMouseMovement(double xpos, double ypos, bool constrainPitch) {
    if (!m_cameraMode) {
        return;
    }

    if (m_firstMouse) {
        m_lastX = xpos;
        m_lastY = ypos;
        m_firstMouse = false;
    }

    float xoffset = xpos - m_lastX;
    float yoffset = m_lastY - ypos; // Reversed since y-coordinates go from bottom to top

    m_lastX = xpos;
    m_lastY = ypos;

    xoffset *= m_mouseSensitivity;
    yoffset *= m_mouseSensitivity;

    m_yaw += xoffset;
    m_pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (m_pitch > 89.0f)
            m_pitch = 89.0f;
        if (m_pitch < -89.0f)
            m_pitch = -89.0f;
    }

    // Update Front, Right and Up vectors using the updated Euler angles
    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
    m_zoom -= yoffset;
    if (m_zoom < 1.0f)
        m_zoom = 1.0f;
    if (m_zoom > 45.0f)
        m_zoom = 45.0f;
}

void Camera::UpdateCameraVectors() {
    // Calculate the new front vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);
    
    // Also re-calculate the right and up vector
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}