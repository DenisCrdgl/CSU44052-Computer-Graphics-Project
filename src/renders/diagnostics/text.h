#pragma once

#include "renders/shader/shader.h"

#include <string>
#include <cstdint>
#include <glm/glm.hpp>

class RenderText{
    public:
        uint32_t vao = 0;
        uint32_t vbo = 0;

        RenderText();
        ~RenderText();

        RenderText(const RenderText&) = delete;
        RenderText& operator=(const RenderText&) = delete;

        void drawTxt(
            const Shader& shader,
            const std::string& txt,
            float x, float y, float scale, 
            const glm::vec3& color,
            int viewportWidth,
            int viewportHeight) const;

        static float txtWidth(const std::string& txt, float scale);
};