#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <cstdint>

struct PropModel{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal;
};

class Gltf{
    public:
        struct Primitive{
            uint32_t vao = 0;
            uint32_t vbo =0;
            uint32_t ebo = 0;
            int indxCount = 0;
        };

        std::vector<Primitive> primitives;

        Gltf() = default;
        Gltf(const Gltf&) = delete;
        Gltf& operator=(const Gltf&) = delete;
        ~Gltf();

        bool loadModel(const std::string& path);
        void drawModel() const;
};