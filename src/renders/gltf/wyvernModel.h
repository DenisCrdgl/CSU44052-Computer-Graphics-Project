#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>

struct Vertex{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec4 weights;
    glm::vec4 joints;
};

struct Binder{
    glm::vec3 translation{0.0f};
    glm::vec3 scale{1.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    std::vector<int> children;
};

struct VecFrame{
    glm::vec3 val{0.0f};
    float time = 0.0f;
};

struct QuaterFrame{
    glm::quat val{1.0f, 0.0f, 0.0f, 0.0f};
    float time = 0.0f;
};

struct VecFrameList{
    std::vector<VecFrame> frames;
    bool step = false;
};

struct QuaterFrameList{
    std::vector<QuaterFrame> frames;
    bool step = false;
};

struct Animation{
    std::unordered_map<int, VecFrameList> translations;
    std::unordered_map<int, QuaterFrameList> rotations;
    std::unordered_map<int, VecFrameList> scales;
    float length = 0.0f;
};

class WyvernModel{
    public:
        struct Primitive{
            uint32_t vao = 0;
            uint32_t vbo = 0;
            uint32_t ebo = 0;
            int indxCount = 0;
        };
        std::vector<Primitive> primitives;

        std::vector<int> roots;
        std::vector<int> joints;
        std::vector<Binder> bindings;
        std::vector<glm::mat4> inverseBindings;
        std::unordered_map<std::string, Animation> animations;
        int meshIndx = -1;

        WyvernModel() = default;
        ~WyvernModel();
        WyvernModel(const WyvernModel&) = delete;
        WyvernModel& operator=(const WyvernModel&) = delete;

        std::vector<glm::mat4> jointMatrices(float length, const std::string& animation) const;
        bool loadWyvern(const std::string& path);
        void drawWyvern() const;
};