#include "scene/user/camera/camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

Camera::Camera(const glm::vec3& position) : pos(position){

}

void Camera::updateCameraDirection(){
    float camPitch = glm::radians(pitch);
    float camYaw = glm::radians(yaw);

    glm::vec3 camPosition;
    camPosition.x = std::cos(camYaw) * std::cos(camPitch);
    camPosition.y = std::sin(camPitch);
    camPosition.z = std::sin(camYaw) * std::cos(camPitch);

    camFront = glm::normalize(camPosition);
    camRight = glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f)));
    camUp = glm::normalize(glm::cross(camRight, camFront));
}

void Camera::processKeys(
    bool up,
    bool down,
    bool left,
    bool right,
    bool forward,
    bool backward,
    float speedRate){
    
    float velocity = speed * speedRate;

    if(up){
        pos += glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
    }
    if(down){
        pos -= glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
    }

    if(left){
        pos -= camRight * velocity;
    }
    if(right){
        pos += camRight * velocity;
    }

    if(forward){
        pos += camFront * velocity;
    }
    if(backward){
        pos -= camFront * velocity;
    }
}

void Camera::processMouse(float xRate, float yRate){
    pitch -= yRate * sens;
    pitch = std::clamp(pitch, -89.0f, 89.0f);
    yaw += xRate * sens;
    updateCameraDirection();
}

glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(pos, pos + camFront, camUp);
}