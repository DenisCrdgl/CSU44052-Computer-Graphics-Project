#include "scene/landscape/clouds/cloud.h"
#include "scene/landscape/clouds/cloudLandscape.h"

#include <cmath>
#include <limits>
#include <algorithm>

/*
    Summary: This class handles the chunks of rendered clouds.
    The landscape generates all of the processed chunks via the function
    genChunkMesh() based on the cloud parameters and builds a cloud for
    every chunk of interest, which then gets merged into MeshData. This uses
    threads for concurrency to process multiple chunks via workerLoop() using
    queues to process closest pending chunks. Chunks are updated based on the 
    position that positionCoords() return, which then updateLandscape() uses
    to choose and derender any chunks outside the agreed derender radius and is
    the main location where pending chunks are queued for workerLoop(). After
    all this, drawLandscape() draws loaded chunks on the scene with
    Frustum culling applied for better performance

    Note: Thread and worker related code is AI generated
*/
CloudLandscape::CloudLandscape(const CloudSeaParams& params)
    : landscapeSize(params.chunkSize * 3.0f), params(params){

    unsigned threads = std::thread::hardware_concurrency();
    unsigned workerNum;
    if(threads > 1){
        workerNum = threads - 1;
    }
    else{
        workerNum = 1;
    }
    workers.reserve(workerNum);
    for(unsigned i = 0; i < workerNum; i++){
        workers.emplace_back(&CloudLandscape::workerLoop, this);
    }
}

CloudLandscape::~CloudLandscape(){
    {
        std::lock_guard<std::mutex> lock(inputMutex);
        exit = true;
    }
    condition.notify_all();
    for(auto& worker : workers){
        if(worker.joinable()){
            worker.join();
        }
    }
}

Coordinates CloudLandscape::positionCoords(const glm::vec3 position, float chunkSize){
    int chunkX = static_cast<int>(std::floor(position.x / chunkSize));
    int chunkZ = static_cast<int>(std::floor(position.z / chunkSize));
    return {chunkX, chunkZ};
}

MeshData CloudLandscape::genChunkMesh(
    const CloudSeaParams& params, 
    Coordinates coords, 
    float chunkSize){

    MeshData mesh;

    float chunkMinX = coords.x * chunkSize;
    int minX = static_cast<int>(std::floor(chunkMinX / params.chunkSize)) - 1;
    float chunkMaxX = chunkMinX + chunkSize;
    int maxX = static_cast<int>(std::ceil(chunkMaxX / params.chunkSize)) + 1;
    float chunkMinZ = coords.z * chunkSize;
    int minZ = static_cast<int>(std::floor(chunkMinZ / params.chunkSize)) - 1;
    float chunkMaxZ = chunkMinZ + chunkSize;
    int maxZ = static_cast<int>(std::ceil(chunkMaxZ / params.chunkSize)) + 1;

    for(int i = minZ; i <= maxZ; i++){
        for(int j = minX; j <= maxX; j++){
            float potentialX = (j + 0.5f) * params.chunkSize;
            int x = static_cast<int>(std::floor(potentialX / chunkSize));
            float potentialZ = (i + 0.5f) * params.chunkSize;
            int z = static_cast<int>(std::floor(potentialZ / chunkSize));

            if(x != coords.x || z != coords.z){
                continue;
            }

            CloudParams cloudParams;
            if(!assembleCloudParams(cloudParams, params, j, i)){
                continue;
            }
            Cloud cloud(cloudParams);
            cloud.addMesh(mesh);
        }
    }

    return mesh;
}

void CloudLandscape::workerLoop(){
    while (true){
        Coordinates coords;

        {
            std::unique_lock<std::mutex> lock(inputMutex);
            condition.wait(lock, [this]{
                return exit || !inputQueue.empty();
            });
            if(exit && inputQueue.empty()){
                return;
            }

            auto pending = inputQueue.begin();
            int pendingDistance = std::numeric_limits<int>::max();
            for(auto i = inputQueue.begin(); i != inputQueue.end(); i++){
                int distanceX = i->x - focus.x;
                int distanceZ = i->z - focus.z;
                int distance = distanceX * distanceX + distanceZ * distanceZ;
                if(distance < pendingDistance){
                    pendingDistance = distance;
                    pending = i;
                }
            }

            coords = *pending;
            inputQueue.erase(pending);
        }

        MeshData mesh = genChunkMesh(params, coords, landscapeSize);

        {
            std::lock_guard<std::mutex> lock(outputMutex);
            outputQueue.push_back({std::move(mesh), coords});
        }
    } 
}

void CloudLandscape::updateLandscape(const glm::vec3& camPos){
    Coordinates cntr = positionCoords(camPos, landscapeSize);
    std::vector<Chunk> updated;

    {
        std::lock_guard<std::mutex> lock(inputMutex);
        focus = cntr;
        for(int i = -renderRadius; i <= renderRadius; i++){
            for(int j = -renderRadius; j <= renderRadius; j++){
                Coordinates temp{cntr.x + j, cntr.z + i};
                if(renderedChunks.count(temp) || renderingChunks.count(temp)){
                    continue;
                }
                inputQueue.push_back(temp);
                renderingChunks.insert(temp);
            }
        }
    }

    condition.notify_all();

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        updated.swap(outputQueue);
    }

    for(auto chunk : updated){
        renderingChunks.erase(chunk.coords);
        int x = chunk.coords.x - cntr.x;
        int z = chunk.coords.z - cntr.z;
        if(std::max(std::abs(x), std::abs(z)) > derenderRadius){
            continue;
        }
        auto mesh = std::make_unique<Mesh>();
        mesh->init(chunk.mesh);
        renderedChunks[chunk.coords] = std::move(mesh);
    }

    for(auto i = renderedChunks.begin(); i != renderedChunks.end();){
        int x = i->first.x - cntr.x;
        int z = i->first.z - cntr.z;
        if(std::max(std::abs(x), std::abs(z)) > derenderRadius){
            i = renderedChunks.erase(i);
        }
        else{
            i++;
        }
    }
}

void CloudLandscape::drawLandscape(const Frustum& frustum) const{
    const float horizontalPadding = 12.0f;
    const float verticalPadding = 12.0f;

    for(const auto& chunk : renderedChunks){
        float chunkMinX = chunk.first.x * landscapeSize;
        float chunkMinZ = chunk.first.z * landscapeSize;
        glm::vec3 minVec(
            chunkMinX - horizontalPadding,
            params.minThresh - verticalPadding,
            chunkMinZ - horizontalPadding
        );
        glm::vec3 maxVec(
            chunkMinX + landscapeSize + horizontalPadding,
            params.maxThresh + verticalPadding,
            chunkMinZ + landscapeSize + horizontalPadding
        );

        if(!frustum.isIntersection(minVec, maxVec)){
            continue;
        }

        chunk.second->drawMesh();
    }
}