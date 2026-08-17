#pragma once

namespace Config{
    constexpr int windowWidth = 1280;
    constexpr int windowHeight = 720;

    constexpr float cameraDistance = 20.0f;
    constexpr float cameraHeight = 10.0f;
    constexpr float cameraFov = 65.0f;
    constexpr float cameraNearPlane = 0.1f;
    constexpr float cameraFarPlane = 1000.0f;

    constexpr int shadowResolution = 1024;

    constexpr float wyvernSpeed = 18.0f;

    constexpr float cloudChunkSize = 15.0f;
    constexpr float cloudSpawnRate = 0.4f;
    constexpr int cloudRenderRadius = 8;
    constexpr int cloudDerenderRadius = 11;

    constexpr float lighthouseChunkSize = 190.0f;
    constexpr float lighthouseSpawnRate = 0.55f;
    constexpr int lighthouseLoadRadius = 2;

    constexpr float balloonChunkSize = 170.0f;
    constexpr float balloonSpawnRate = 0.55f;
    constexpr int balloonLoadRadius = 3;

    constexpr float toriiChunkSize = 150.0f;
    constexpr float toriiSpawnRate = 0.6f;
    constexpr int toriiLoadRadius = 2;

    constexpr float treeChunkSize = 100.0f;
    constexpr float treeSpawnRate = 0.6f;
    constexpr int treeLoadRadius = 3;
}
