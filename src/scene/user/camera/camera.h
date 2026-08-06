#pragma once

#include <glm/glm.hpp>

class Camera {
    public:
    glm::vec3 pos;
    glm::vec3 camFront{0.0f, 0.0f, -1.0f};
    glm::vec3 camUp{0.0f, 1.0f, 0.0f};
    glm::vec3 camRight{1.0f, 0.0f, 0.0f};

    float sens = 0.12f;
    float pitch = 0.0f;
    float yaw = -90.0f;
    float speed = 8.0f;

    const glm::vec3& position() const {
        return pos;
    };

    explicit Camera(const glm::vec3& position);
    glm::mat4 viewMatrix() const;

    void processKeys(
        bool up,
        bool down,
        bool left,
        bool right,
        bool forward,
        bool backward,
        float speedRate
    );
    void processMouse(float xRate, float yRate);
    
    void updateCameraDirection();
};