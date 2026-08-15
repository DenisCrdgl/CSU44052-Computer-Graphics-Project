#pragma once

#include "renders/shader/shader.h"
#include "renders/gltf/model.h"

#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <string>

struct PropParams{
    float minHeight = 18.0f;
    float maxHeight = 46.0f;
    float minScale = 1.0f;
    float maxScale = 1.0f;
    int minNum = 1;
    int maxNum = 1;

    float groupRadius = 0.0f;
    int loadRadiusChunks = 2;
    float chunkSize = 130.0f;
    float spawnRate = 0.6f;

    uint32_t seed = 2026;
};

class Prop{
    public:
        struct PropInstance{
            float yaw;
            float scale;
            glm::vec3 position;
        };

        Gltf model;
        std::string path;

        std::vector<PropInstance> instances;
        PropParams params;

        Prop(const PropParams& propParams, std::string path);

        void sample(std::vector<PropInstance>& result, int x, int z) const;
        bool load();
        void update(const glm::vec3& camPos);
        void drawProp(const Shader& shader) const;
};