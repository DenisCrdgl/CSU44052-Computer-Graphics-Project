#include "renders/shadow/shadow.h"

#include <glad/gl.h>

/*
    Summary: This class handles the shadow depth processing.
    It has a start() function which binds shadow resolution to FBO
    and resizes viewport in addition to cleaning the depth pass buffer 
    in anticipation of the rendering of the model of interest. 
    The complement function stop() unbinds FBO and resizes viewport to
    value specified inputs given to the function. The depth texture is then bound
    via shaders using bind() function which contain the necessary shadowSampling
    functions in their frag files  
*/
Shadow::Shadow(int resolution) : res(resolution){
    glGenTextures(1, &depth);
    glBindTexture(GL_TEXTURE_2D, depth);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT24,
        res, res,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    const float shadowColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, shadowColor);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, 
        GL_DEPTH_ATTACHMENT, 
        GL_TEXTURE_2D, 
        depth, 
        0
    );
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);  
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Shadow::~Shadow(){
    if(depth){
        glDeleteTextures(1, &depth);
    }
    if(fbo){
        glDeleteFramebuffers(1, &fbo);
    }
}

void Shadow::start() const{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, res, res);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void Shadow::stop(int viewportWidth, int viewportHeight) const{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewportWidth, viewportHeight);
}

void Shadow::bind(int bindSlot) const{
    glActiveTexture(GL_TEXTURE0 + bindSlot);
    glBindTexture(GL_TEXTURE_2D, depth);
}