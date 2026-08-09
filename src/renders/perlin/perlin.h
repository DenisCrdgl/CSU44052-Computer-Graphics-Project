#pragma once

#include <cstdint>

class Perlin {
    public:
        uint8_t permutationTable[512];

        explicit Perlin(uint32_t seed = 2026);

        static float gradient(int hash, float x, float y, float z);
        static float smooth(float cellDist);
        static float blend(float smoothVal, float a, float b);
        float generate(float x, float y, float z) const;
        float assemble(
            float x, float y, float z, 
            int octaves = 4, 
            float lacunarity = 2.0f,
            float gain = 0.5f
        );
};