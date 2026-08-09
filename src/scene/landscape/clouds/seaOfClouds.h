#pragma once

#include <glm/glm.hpp>
#include <cstdint>

struct CloudSeaParams {
    uint32_t seed = 2026;
    float spawnRate = 0.52f;
    float chunkSize = 15.0f;
    float maxThresh = 26.0f;
    float minThresh = 8.0f;
};

bool cloudChunk(
    glm::vec3& position,
    const CloudSeaParams& params,
    int xRate,
    int zRate
);