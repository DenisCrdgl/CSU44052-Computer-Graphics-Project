#include "scene/landscape/clouds/seaOfClouds.h"
#include "renders/perlin/perlin.h"

#include <glm/glm.hpp>
#include <random>

namespace {
    uint32_t hashedChunk(int x, int z, uint32_t seed){
        uint32_t hashedX = static_cast<uint32_t>(x) * 72290103u;
        uint32_t hashedZ = static_cast<uint32_t>(z) * 23329993u;
        uint32_t hash = hashedX ^ hashedZ ^ seed;
        hash ^= hash >> 16;
        hash *= 0x9aaa777d;
        hash ^= hash >> 15;
        hash *= 0x777abcba;
        hash ^= hash >> 16;
        return hash;
    }
}

/*
    Summary: This file controls the cloud chunk rendering.
    The cloudChunk() function decides the spawn points of clouds per chunk,
    using Perlin noise sampling for height (gated by a threshold) and rng for
    x/z location coordinates. The assembleCloudParams() function assembles the
    necessary parameters to generate the clouds on these spawn points,
    additionally using rng to decide frequency, spread and amplitude for
    per cloud uniqueness
*/
bool cloudChunk(glm::vec3& position, const CloudSeaParams& params, int x, int z){
    Perlin layout(params.seed);

    float chunkX = (x + 0.5f) * params.chunkSize;
    float chunkZ = (z + 0.5f) * params.chunkSize;
    float temp = layout.assemble(chunkX * 0.045f, chunkZ * 0.045f, 0.0f, 3);
    float seaDensity = glm::clamp(temp * 0.5f + 0.5f, 0.0f, 1.0f);

    if (seaDensity < params.spawnRate)
    {
        return false;
    }

    temp = layout.assemble(chunkX * 0.07f + 500.0f, chunkZ * 0.07f + 500.0f, 0.0f, 3);
    float seaHeight = glm::clamp(temp * 0.5f + 0.5f, 0.0f, 1.0f);
    uint32_t chunkSeed = hashedChunk(x, z, params.seed);
    std::mt19937 rng(chunkSeed);
    std::uniform_real_distribution<float> jitter(-1.0f, 1.0f);

    position = glm::vec3(
        chunkX + jitter(rng) * params.chunkSize * 0.35f,
        glm::mix(params.minThresh, params.maxThresh, seaHeight),
        chunkZ + jitter(rng) * params.chunkSize * 0.35f);

    return true;
}

bool assembleCloudParams(CloudParams& result, const CloudSeaParams& params, int x, int z){
    glm::vec3 position;

    if(!cloudChunk(position, params, x, z)){
        return false;
    }

    uint32_t seed = hashedChunk(x, z, params.seed);
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> randFloat(0.0f, 1.0f);

    result.position = position;
    result.seed = seed;
    result.cloudFrequency = 0.5f + randFloat(rng) * 0.25f;
    result.cloudSpread = 2.5f + randFloat(rng) * 3.0f;
    result.cloudAmplitude = result.cloudSpread * 0.35f;
    result.cloudHeight = 0.25f + randFloat(rng) * 0.15f;
    result.cloudFluff = 4 + static_cast<int>(randFloat(rng) * 3.0f);
    result.cloudFragments = 10;

    return true;
}
