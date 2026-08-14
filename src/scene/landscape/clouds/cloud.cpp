#include "scene/landscape/clouds/cloud.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <random>
#include <limits>


namespace{
    struct Sphere{
        std::vector<uint32_t> indices;
        std::vector<glm::vec3> directions;
    };

    Sphere buildSphere(int segments){
        Sphere top;

        for(int i = 0; i <= segments; i++){
            float theta = 
                static_cast<float>(i) / 
                static_cast<float>(segments) * 
                glm::pi<float>();

            for(int j = 0; j <= segments * 2; j++ ){
                float phi = 
                    static_cast<float>(j) /
                    static_cast<float>(segments * 2) *
                    glm::two_pi<float>();

                    top.directions.push_back(
                        glm::vec3(
                            std::sin(theta) * std::cos(phi),
                            std::cos(theta),
                            std::sin(theta) * std::sin(phi)
                        )
                    );
            }   
        }

        int stride = segments * 2 + 1;
        for(int i = 0; i < segments; i++){
            uint32_t currVert = static_cast<uint32_t>(i * stride);
            uint32_t nextVert = currVert + static_cast<uint32_t>(stride);

            for(int j = 0; j <= segments * 2; j++, currVert++, nextVert++){
                
                if(i != 0){
                    top.indices.push_back(currVert);
                    top.indices.push_back(nextVert);
                    top.indices.push_back(currVert + 1);
                }

                if(i != segments - 1){
                    top.indices.push_back(currVert + 1);
                    top.indices.push_back(nextVert);
                    top.indices.push_back(nextVert + 1);
                }
            }
        }

        return top;
    }
}

Cloud::Cloud(const CloudParams& cloudParams): 
    params(cloudParams), perlin(cloudParams.seed){
    
    std::mt19937 rng(cloudParams.seed);
    std::uniform_real_distribution<float> posNegRange(-1.0f, 1.0f);
    std::uniform_real_distribution<float> posRange(0.0f, 1.0f);
    perlinOffset = glm::vec3(posNegRange(rng), posNegRange(rng), posNegRange(rng)) * 1000.0f;

    fluffs.push_back({params.cloudSpread, glm::vec3(0.0f)});
    for(int i= 1; i < params.cloudFluff; i++){
        glm::vec3 offset(
            posNegRange(rng) * params.cloudSpread * 0.6f,
            posNegRange(rng) * params.cloudSpread * 0.45f,
            posNegRange(rng) * params.cloudSpread * 0.6f
        );
        float radius = params.cloudSpread * (0.5f + posRange(rng) * 0.6f);
        fluffs.push_back({radius, offset});
    }

    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    for(const auto& fluff : fluffs){
        minY = glm::min(minY, fluff.offset.y - fluff.radius);
        maxY = glm::max(maxY, fluff.offset.y + fluff.radius);
    }

    yHeight = minY + (maxY - minY) * params.cloudHeight;
}

void Cloud::addMesh(MeshData& result){
    Sphere top = buildSphere(params.cloudFragments);

    for(const auto& fluff : fluffs){
        uint32_t indxOffset = static_cast<uint32_t>(result.vertices.size());
        std::vector<glm::vec3> fluffTexVec(top.directions.size());
        for(int i = 0; i < top.directions.size(); i++){
            glm::vec3 direction = top.directions[i];
            glm::vec3 cloudRelativePos = fluff.offset + direction * fluff.radius;
            glm::vec3 worldRelativePos = cloudRelativePos + params.position;
            glm::vec3 sample = (worldRelativePos + perlinOffset) * params.cloudFrequency;
            float genPerlin = perlin.assemble(sample.x, sample.y, sample.z, 3);

            glm::vec3 fluffTex = 
                cloudRelativePos + direction * genPerlin * params.cloudAmplitude;
            if(fluffTex.y < yHeight){
                fluffTex.y = yHeight;
            }
            fluffTexVec[i] = fluffTex;
        }

        std::vector<glm::vec3> normals(fluffTexVec.size(), glm::vec3(0.0f));
        for(int i = 0; i < top.indices.size(); i += 3){
            uint32_t i0 = top.indices[i];
            uint32_t i1 = top.indices[i + 1];
            uint32_t i2 = top.indices[i + 2];

            glm::vec3 surfaceNormal = glm::cross(
                fluffTexVec[i1] - fluffTexVec[i0],
                fluffTexVec[i2] - fluffTexVec[i0]
            );
            normals[i0] += surfaceNormal;
            normals[i1] += surfaceNormal;
            normals[i2] += surfaceNormal;
        }

        for(int i = 0; i < fluffTexVec.size(); i++){
            glm::vec3 normalVec;
            if(glm::length(normals[i]) > 1e-8f){
                normalVec = glm::normalize(normals[i]);
            }
            else{
                normalVec = top.directions[i];
            }
            result.vertices.push_back({normalVec, params.position + fluffTexVec[i]});
        }

        for(uint32_t indx : top.indices){
            result.indices.push_back(indxOffset + indx);
        }
    }
}