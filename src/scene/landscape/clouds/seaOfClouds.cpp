#include "scene/landscape/clouds/seaOfClouds.h"
#include "renders/perlin/perlin.h"

#include <glm/glm.hpp>
#include <random>

namespace {
    uint32_t hashedChunk(int xRate, int zRate, uint32_t seed){
        uint32_t hashedX = static_cast<uint32_t>(xRate) * 72290103u;
        uint32_t hashedZ = static_cast<uint32_t>(zRate) * 23329993u;
        uint32_t hash = hashedX ^ hashedZ ^ seed;
        hash ^= hash >> 16;
        hash *= 0x9aaa777d;
        hash ^= hash >> 15;
        hash *= 0x777abcba;
        hash ^= hash >> 16;
        return hash;
    }
}

bool cloudChunk(glm::vec3& position, const CloudSeaParams& params, int xRate, int zRate){
    Perlin layout(params.seed);

    float chunkX = (xRate + 0.5f) * params.chunkSize;
    float chunkZ = (zRate + 0.5f) * params.chunkSize;
    float temp = layout.assemble(chunkX * 0.045f, chunkZ * 0.045f, 0.0f, 3);
    float seaDensity = glm::clamp(temp * 0.5f + 0.5f, 0.0f, 1.0f);

    if (seaDensity < params.spawnRate)
    {
        return false;
    }

    temp = layout.assemble(chunkX * 0.07f + 500.0f, chunkZ * 0.07f + 500.0f, 0.0f, 3);
    float seaHeight = glm::clamp(temp * 0.5f + 0.5f, 0.0f, 1.0f);
    uint32_t chunkSeed = hashedChunk(xRate, zRate, params.seed);
    std::mt19937 rng(chunkSeed);
    std::uniform_real_distribution<float> jitter(-1.0f, 1.0f);

    // for later
    position = glm::vec3(
        chunkX + jitter(rng) * params.chunkSize * 0.35f,
        glm::mix(params.minThresh, params.maxThresh, seaHeight),
        chunkZ + jitter(rng) * params.chunkSize * 0.35f);

    return true;
}
