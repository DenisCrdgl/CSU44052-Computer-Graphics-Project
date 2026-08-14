#pragma once

#include "scene/landscape/clouds/seaOfClouds.h"
#include "renders/mesh/mesh.h"
#include "renders/frustum/frustum.h"

#include <glm/glm.hpp>
#include <vector>
#include <cstddef>
#include <memory>
#include <mutex>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <condition_variable>
#include <thread>
#include <functional>

struct Coordinates{
    int x;
    int z;

    bool operator==(const Coordinates& point) const{
        return x == point.x && z == point.z;
    }
};

struct HashedCoordinates{
    size_t operator()(const Coordinates& coords) const{
        size_t hashX = std::hash<int>()(coords.x);
        size_t hashZ = std::hash<int>()(coords.z);
        size_t result = hashX ^ (hashZ + 0x9e3779b9u + (hashX << 6) + (hashX >> 2));
        return result;
    }
};

class CloudLandscape{
    public:
        struct Chunk{
            MeshData mesh;
            Coordinates coords;
        };

        bool exit = false;
        float landscapeSize;
        int renderRadius = 15;
        int derenderRadius = 18;
        CloudSeaParams params;

        std::mutex inputMutex;
        std::mutex outputMutex;
        std::condition_variable condition;
        std::deque<Coordinates> inputQueue;
        std::vector<Chunk> outputQueue;
        std::vector<std::thread> workers;
        Coordinates focus{0, 0};

        std::unordered_map< 
            Coordinates, 
            std::unique_ptr<Mesh>,
            HashedCoordinates
        > renderedChunks;
        std::unordered_set<
            Coordinates, 
            HashedCoordinates
        > renderingChunks;

        explicit CloudLandscape(const CloudSeaParams& params = CloudSeaParams());
        CloudLandscape(const CloudLandscape&) = delete;
        ~CloudLandscape();
        CloudLandscape& operator=(const CloudLandscape&) = delete;

        void updateLandscape(const glm::vec3& camPos);
        void drawLandscape(const Frustum& frustum) const;
        int lodedChunks() const {
            return renderedChunks.size();
        }
        static Coordinates positionCoords(const glm::vec3 position, float chunkSize);
        static MeshData genChunkMesh(const CloudSeaParams& params, Coordinates coords, float chunkSize);
        void workerLoop();

};