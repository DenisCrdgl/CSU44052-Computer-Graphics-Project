#include "scene/user/camera/camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Camera::Camera(const glm::vec3& position) : pos(position){

}

glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(pos, pos + camFront, camUp);
}

void Camera::thirdPerson(
    float distance,
    float height,
    float frameTime,
    const glm::vec3& modelPos,
    const glm::vec3& modelForward,
    const glm::vec3& modelUp){

    glm::vec3 thirdPersonPos = 
        modelPos - modelForward * 
        distance + modelUp * 
        height;
    float smoothen = 1.0f - std::exp(-6.0f * frameTime);
    pos = glm::mix(pos, thirdPersonPos, smoothen);
    glm::vec3 lookAt = modelPos + modelForward * 4.0f;
    camFront = glm::normalize(lookAt - pos);
    camRight = glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f)));
    camUp = glm::normalize(glm::cross(camRight, camFront));
}