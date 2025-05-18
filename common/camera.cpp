#include <common/camera.hpp>

Camera::Camera(const glm::vec3 Eye, const glm::vec3 Target)
{
    eye = Eye;
    target = Target;
}

void Camera::calculateMatrices()
{
    calculateCameraVectors();
    // Calculate the view matrix
    view = glm::lookAt(eye, eye + front, worldUp);

    // Calculate the projection matrix
    projection = glm::perspective(fov, aspect, near, far);
}
void Camera::calculateCameraVectors()
{
    front = glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::cross(right, front);
}
