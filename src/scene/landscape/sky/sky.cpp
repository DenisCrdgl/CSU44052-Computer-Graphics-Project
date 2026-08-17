#include "scene/landscape/sky/sky.h"

#include <glad/gl.h>

namespace {
    const float quad[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, 1.0f,
        -1.0f, 1.0f,
    };
}

/*
    Summary: Draws the gradient sky and sun using
    drawSky() function. Most of the functionality
    is based in the vert/frag files for the sky which
    define the gradient based on the view, making the sky
    slightly shift color the farther up you look 
*/
Sky::Sky() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

Sky::~Sky(){
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void Sky::drawSky() const {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}