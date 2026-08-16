#pragma once

#include <glm/glm.hpp>

class Camera {
    public:
    glm::vec3 pos;
    glm::vec3 camFront{0.0f, 0.0f, -1.0f};
    glm::vec3 camUp{0.0f, 1.0f, 0.0f};
    glm::vec3 camRight{1.0f, 0.0f, 0.0f};

    const glm::vec3& position() const {
        return pos;
    };

    explicit Camera(const glm::vec3& position);
    glm::mat4 viewMatrix() const;
    
    void thirdPerson(
        float distance,
        float height,
        float frameTime,
        const glm::vec3& modelPos,
        const glm::vec3& modelForward,
        const glm::vec3& modelUp
    );
};