#include "scene/landscape/props/props.h"
#include "renders/perlin/perlin.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <random>

namespace{
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

    int positionCoords(float size, float coord){
        return static_cast<int>(std::floor(coord / size));
    }
}

Prop::Prop(const PropParams& propParams, std::string path)
    : path(std::move(path)), params(propParams){
}

void Prop::sample(std::vector<PropInstance>& result, int x, int z) const{
    Perlin layout(params.seed);
    float chunkX = params.chunkSize * (x + 0.5f);
    float chunkZ = params.chunkSize * (z + 0.5f);
    float perlin = layout.assemble(
        chunkX * 0.02f + 0.1373f,
        chunkZ * 0.02f + 0.2917f,
        0.0f,
        3
    );
    float propFrequency = glm::clamp(perlin * 0.5f + 0.5f, 0.0f, 1.0f);

    if(propFrequency < params.spawnRate){
        return;
    }

    uint32_t seed = hashedChunk(x, z, params.seed);
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> posRange(0.0f, 1.0f);
    std::uniform_real_distribution<float> angle(0.0f, 360.0f);
    std::uniform_real_distribution<float> jitter(-1.0f, 1.0f);

    int propNum = params.minNum;
    if(params.maxNum > params.minNum){
        propNum = 
            params.minNum + 
            static_cast<int>(posRange(rng) *
            static_cast<float>(params.maxNum - params.minNum + 1));
        propNum = glm::clamp(propNum, params.minNum, params.maxNum);
    }

    for(int i = 0; i < propNum; i++){
        float scatterX;
        float scatterZ;
        if(propNum > 1){
            float perPropSpaceAngle = (360.0f / static_cast<float>(propNum)) *
                static_cast<float>(i) + jitter(rng) * 20.0f;
            float perPropSpaceRadius = params.groupRadius * (0.75f + 0.35f * posRange(rng));
            
            scatterX = std::cos(glm::radians(perPropSpaceAngle)) * perPropSpaceRadius;
            scatterZ = std::sin(glm::radians(perPropSpaceAngle)) * perPropSpaceRadius;
        }
        else{
            scatterX = jitter(rng) * params.chunkSize * 0.3f;
            scatterZ = jitter(rng) * params.chunkSize * 0.3f;   
        }

        PropInstance instance;
        instance.yaw = angle(rng);
        instance.scale = glm::mix(params.minScale, params.maxScale, posRange(rng));
        instance.position = glm::vec3(
            chunkX + scatterX,
            glm::mix(params.minHeight, params.maxHeight, posRange(rng)),
            chunkZ + scatterZ
        );
        result.push_back(instance);
    }
}

bool Prop::load(){
    return model.loadModel(path);
}

void Prop::update(const glm::vec3& camPos){
    int x = positionCoords(params.chunkSize, camPos.x);
    int z = positionCoords(params.chunkSize, camPos.z);
    instances.clear();
    for(int i = -params.loadRadiusChunks; i <= params.loadRadiusChunks; i++){
        for(int j = -params.loadRadiusChunks; j <= params.loadRadiusChunks; j++){
            sample(instances, x + j, z + i);
        }
    }
}

void Prop::drawProp(const Shader& shader) const{
    for(const auto& instance : instances){
        glm::mat4 modelMat = glm::translate(glm::mat4(1.0f), instance.position);
        modelMat = glm::rotate(modelMat, glm::radians(instance.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMat = glm::scale(modelMat, glm::vec3(instance.scale));
        shader.uniSet("model", modelMat);
        model.drawModel();
    }
}