#pragma once

#include "renders/shader/shader.h"
#include "renders/gltf/wyvernModel.h"

#include <glm/glm.hpp>
#include <string>

class Wyvern{
    public:
        WyvernModel model;

        float pitch = 0.0f;
        float yaw = -90.0f;
        float animationTime = 0.0f;
        float sensitivity = 0.15f;
        float speed = 18.0f;
        float maxPitch = 80.0f;

        glm::vec3 pos;
        glm::vec3 up{0.0f, 1.0f, 0.0f};
        glm::vec3 right{1.0f, 0.0f, 0.0f};
        glm::vec3 forward{0.0f, 0.0f, -1.0f};
        std::string flightAnimation = "Stationary_Flight";

        explicit Wyvern(const glm::vec3& position);

        glm::mat4 modelMtrx() const;
        void updateVec();
        void processMouseInput(float x, float y);
        void processKeyInput(bool wKey, bool sKey, bool aKey, bool dKey, float frameTime);
        bool loadWyvern(const std::string& path);
        void drawWyvern(const Shader& shader, const glm::vec3& color) const;
        void drawShadow(const Shader& shader) const;
};