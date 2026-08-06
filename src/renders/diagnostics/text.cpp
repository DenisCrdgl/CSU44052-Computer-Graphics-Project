#include "renders/diagnostics/text.h"

#include <glad/gl.h>

#include <vector>
#include <array>
#include <unordered_map>

namespace{
    using Glyph = std::array<uint8_t, 7>;

    const std::unordered_map<char, Glyph>& glyphTable() {
        static const std::unordered_map<char, Glyph> table = {
            {'F', {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000}},
            {'P', {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000}},
            {'S', {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110}},
            {':', {0b00000, 0b01100, 0b01100, 0b00000, 0b01100, 0b01100, 0b00000}},
            {'0', {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110}},
            {'1', {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}},
            {'2', {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111}},
            {'3', {0b01110, 0b10001, 0b00001, 0b00110, 0b00001, 0b10001, 0b01110}},
            {'4', {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010}},
            {'5', {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110}},
            {'6', {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110}},
            {'7', {0b11111, 0b00001, 0b00010, 0b00100, 0b00100, 0b00100, 0b00100}},
            {'8', {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110}},
            {'9', {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100}},
            {'X', {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001}},
            {'Y', {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100}},
            {'Z', {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111}},
            {' ', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}},
            {'-', {0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000}},
            {',', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b01100, 0b01000}},
        };
        return table;
    }

    constexpr int glyphCols = 5;
    constexpr int glyphRows = 7;
    constexpr float glyphSpacing = 1.0f;

    float scalarUpdate(float scale) {
        return (glyphCols + glyphSpacing) * scale;
    }
}

RenderText::RenderText(){
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glBindVertexArray(0);
}

RenderText::~RenderText(){
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

float RenderText::txtWidth(const std::string& txt, float scale){
    if(txt.empty()){
        return 0.0f;
    }
    return static_cast<float>(txt.size()) * scalarUpdate(scale) - glyphSpacing * scale;
}

void RenderText::drawTxt(
        const Shader& shader,
        const std::string& txt,
        float x, float y, float scale,
        const glm::vec3& color,
        int viewportWidth,
        int viewportHeight) const{

    const auto& displayTable = glyphTable();

    std::vector<float> vertices;
    vertices.reserve(txt.size() * glyphCols * glyphRows * 12);

    float painter = x;
    for(char letter : txt){
        char caseFixedLetter = (letter >= 'a' && letter <= 'z') ?
            static_cast<char>(letter - 'a' + 'A') : letter;
        auto matchedLetter = displayTable.find(caseFixedLetter);
        if(matchedLetter != displayTable.end()){
            const Glyph& glyph = matchedLetter->second;
            for(int row = 0; row < glyphRows; row++){
                uint8_t bits = glyph[row];
                for(int col = 0; col < glyphCols; col++){
                    if(!(bits & (1 << (glyphCols - 1 - col)))){
                        continue;
                    }

                    float x0 = painter + col * scale;
                    float y0 = y + row * scale;
                    float x1 = x0 + scale;
                    float y1 = y0 + scale;

                    float quad[12] = {
                        x0, y0, x1, y0, x1, y1,
                        x0, y0, x1, y1, x0, y1
                    };

                    vertices.insert(vertices.end(), quad, quad + 12);
                }
            }
        }

        painter += scalarUpdate(scale);
    }

    if(vertices.empty()){
        return;
    }

    shader.uniSet("uViewportSize", glm::vec2(static_cast<float>(viewportWidth), static_cast<float>(viewportHeight)));
    shader.uniSet("uColor", color);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 2));
    glBindVertexArray(0);
}