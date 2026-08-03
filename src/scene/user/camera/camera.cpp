#include "scene/user/camera/camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const glm::vec3& position) : pos(position){}

glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(pos, pos + camFront, camUp);
}