#pragma once

#include <cstdint>

class Sky{
    public:
        uint32_t vao = 0;
        uint32_t vbo = 0;

        Sky();
        ~Sky();
        Sky(const Sky&) = delete;
        Sky& operator=(const Sky&) = delete;

        void drawSky() const;
};