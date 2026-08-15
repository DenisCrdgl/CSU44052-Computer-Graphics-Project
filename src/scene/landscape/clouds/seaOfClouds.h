#pragma once

#include "scene/landscape/clouds/cloud.h"

#include <glm/glm.hpp>
#include <cstdint>

struct CloudSeaParams {
    uint32_t seed = 2026;
    float spawnRate = 0.4f;
    float chunkSize = 15.0f;
    float maxThresh = 26.0f;
    float minThresh = 8.0f;
};

bool cloudChunk(
    glm::vec3& position,
    const CloudSeaParams& params,
    int x,
    int z
);

bool assembleCloudParams(
    CloudParams& result,
    const CloudSeaParams& params,
    int x,
    int z
);