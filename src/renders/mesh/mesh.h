#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

struct Vert{
    glm::vec3 normal;
    glm::vec3 position;
};

struct MeshData{
    std::vector<Vert> vertices;
    std::vector<uint32_t> indices;
};

class Mesh{
    public:
        int indxNum = 0;
        uint32_t vao = 0;
        uint32_t vbo = 0;
        uint32_t ebo = 0;

        Mesh() = default;
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        void init(const MeshData& mesh);
        void drawMesh() const;
};