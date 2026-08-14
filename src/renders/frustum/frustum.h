#pragma once

#include <glm/glm.hpp>
#include <array>

class Frustum{
    public:
        std::array<glm::vec4, 6> planes;

        explicit Frustum(const glm::mat4& viewProjection);
        bool isIntersection(const glm::vec3& min, const glm::vec3& max) const;
};