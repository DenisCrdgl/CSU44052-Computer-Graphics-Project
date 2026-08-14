#pragma once

#include "renders/perlin/perlin.h"
#include "renders/mesh/mesh.h"

#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

struct CloudParams{
    glm::vec3 position{0.0f};
    uint32_t seed = 5271;
    float cloudFrequency = 0.55f;
    float cloudSpread = 2.5f;
    float cloudAmplitude = 0.85f;
    float cloudHeight = 0.35f;
    int cloudFluff = 8;
    int cloudFragments = 8;
};

class Cloud{
    public:
        Perlin perlin;
        glm::vec3 perlinOffset{0.0f};

        CloudParams params;
        float yHeight = 0.0f;        

        struct Fluff{
            float radius;
            glm::vec3 offset;
        };

        std::vector<Fluff> fluffs;

        explicit Cloud(const CloudParams& params);
        void addMesh(MeshData& result);
};