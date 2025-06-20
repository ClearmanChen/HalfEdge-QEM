#include "Camera.h"

Camera::Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
float yaw = YAW, float pitch = PITCH, int sc_width = 800, int sc_height = 600) :
    Front(glm::vec3(0.0f, 0.0f, -1.0f)), 
    MovementSpeed(SPEED), 
    MouseSensitivity(SENSITIVITY), 
    Zoom(ZOOM)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    screenWidth = sc_width;
    screenHeight = sc_height;
    ifFixed = false;
    Focus = glm::vec3(0.f, 0.f, 0.f);
    updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, 
    float yaw, float pitch, int sc_width = 800, int sc_height = 600) :
    Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    screenWidth = sc_width;
    screenHeight = sc_height;
    ifFixed = false;
    Focus = glm::vec3(0.f, 0.f, 0.f);
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::GetProjectionMatrix()
{
    return glm::perspective(glm::radians(Zoom), screenWidth / (float)screenHeight, 0.1f, 100.0f);
}

void Camera::setShader(const Shader& shader)
{
    shader.setMat4("projection", GetProjectionMatrix());
    shader.setMat4("view", GetViewMatrix());
    shader.setVec3("viewPos", Position);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    if (ifFixed) return;
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    if (ifFixed) return;
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    if (fabs(xoffset) < 1e-6 && fabs(yoffset) < 1e-6) return;
    auto target = Focus - Position;
    auto right = glm::normalize(glm::cross(target, WorldUp));
    auto up = glm::normalize(glm::cross(right, target));
    float radius = glm::length(target);
    

    auto offset = right * xoffset + up * yoffset;
    float offset_len = glm::length(offset);
    float angle = -offset_len / radius;

    offset = glm::normalize(offset);
    target = glm::normalize(target);
    Position = Focus + (-target * glm::cos(glm::vec3(angle)) + offset * glm::sin(glm::vec3(angle))) * radius;
    Front = glm::normalize(Focus - Position);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(right, Front));
}

void Camera::ProcessMouseScroll(float yoffset)
{
    if (ifFixed) return;
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

void Camera::printState()
{
    std::cout << "Position:" << Position.x << " " << Position.y << " " << Position.z << std::endl;
    std::cout << "Front:" << Front.x << " " << Front.y << " " << Front.z << std::endl;
    std::cout << "Orien:" << Yaw << " " << Pitch << std::endl;
}

void Camera::setPose(float posX, float posY, float posZ, float yaw, float pitch)
{
    if (ifFixed) return;
    Position.x = posX;
    Position.y = posY;
    Position.z = posZ;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

void Camera::setFix(bool fixed)
{
    ifFixed = fixed;
}

void Camera::updateCameraVectors()
{
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up = glm::normalize(glm::cross(Right, Front));
}

