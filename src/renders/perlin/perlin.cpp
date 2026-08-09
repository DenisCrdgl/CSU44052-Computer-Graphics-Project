#include "renders/perlin/perlin.h"

#include <random>
#include <cmath>
#include <algorithm>
#include <numeric>

Perlin::Perlin(uint32_t seed){
    uint8_t base[256];
    std::iota(std::begin(base), std::end(base), 0);
    std::mt19937 rng(seed);
    std::shuffle(std::begin(base), std::end(base), rng);

    for(int i = 0; i < 256; i++){
        permutationTable[i] = base[i];
        permutationTable[i + 256] = base[i];
    }
}

float Perlin::gradient(int hash, float x, float y, float z){
    int gradHash = hash & 15;
    
    float u;
    if(gradHash < 8){
        u = x;
    }
    else{
        u = y;
    }

    float v;
    if(gradHash < 4){
        v = y;
    }
    else{
        if(gradHash == 12 || gradHash == 14){
            v = x;
        }
        else{
            v = z;
        }
    }

    if((gradHash & 1) != 0){
        u *= -1;
    }
    if((gradHash & 2) != 0){
        v *= -1;
    }

    return u + v;
}

float Perlin::smooth(float cellDist){
    return cellDist * cellDist * cellDist * 
        (cellDist * (cellDist * 6.0f - 15.0f) + 10.0f);
}

float Perlin::blend(float smoothVal, float a, float b){
    return a + smoothVal * (b - a);
}

float Perlin::generate(float x, float y, float z) const {
    int hashX = static_cast<int>(std::floor(x)) & 255;
    x -= std::floor(x);
    float u = smooth(x);

    int hashY = static_cast<int>(std::floor(y)) & 255;
    y -= std::floor(y);
    float v = smooth(y);

    int hashZ = static_cast<int>(std::floor(z)) & 255;
    z -= std::floor(z);
    float w = smooth(z);

    int pointA = permutationTable[hashX] + hashY;
    int pointB = permutationTable[hashX + 1] + hashY;

    int pointA2 = permutationTable[pointA] + hashZ;
    int pointB2 = permutationTable[pointB + 1] + hashZ;

    int pointAB = permutationTable[pointA + 1] + hashZ;
    int pointBA = permutationTable[pointB] + hashZ;

    return blend(
        w,
        blend(
            v,
            blend(
                u, 
                gradient(permutationTable[pointA2], x, y, z), 
                gradient(permutationTable[pointBA], x-1, y, z)
            ),
            blend(
                u,
                gradient(permutationTable[pointAB], x, y-1, z),
                gradient(permutationTable[pointB2], x-1, y-1, z)
            )
        ),
        blend(
            v,
            blend(
                u,
                gradient(permutationTable[pointA2+1], x, y, z-1),
                gradient(permutationTable[pointBA+1], x-1, y, z-1)
            ),
            blend(
                u,
                gradient(permutationTable[pointAB+1], x, y-1, z-1),
                gradient(permutationTable[pointB2+1], x-1, y-1, z-1)
            )
        )
    );
}

float Perlin::assemble(float x, float y, float z, int octaves, float lacunarity, float gain){
    float scale = 0.5f;
    float frequency = 1.0f;
    
    float result = 0.0f;
    for(int i = 0; i < octaves; i++){
        result += scale * generate(x * frequency, y * frequency, z * frequency);
        scale *= gain;
        frequency *= lacunarity;
    }

    return result;
}