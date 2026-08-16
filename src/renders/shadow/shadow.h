#pragma once

#include <cstdint>

class Shadow{
    public:
        uint32_t fbo = 0;
        uint32_t depth = 0;
        int res;

        explicit Shadow(int resolution = 1024);
        ~Shadow();
        Shadow(const Shadow&) = delete;
        Shadow& operator=(const Shadow&) = delete;

        void start() const;
        void stop(int viewportWidth, int viewportHeight) const;
        void bind(int bindSlot) const;
};