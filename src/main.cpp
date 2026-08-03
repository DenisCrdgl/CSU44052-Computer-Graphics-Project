#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renders/shader/shader.h"
#include "renders/diagnostics/text.h"

#include "scene/user/camera/camera.h"
#include "scene/landscape/sky/sky.h"

#include <cstdio>

namespace {
    int viewportWidth = 1280;
    int viewportHeight = 720;

    Camera camera(glm::vec3(0.0f, 24.0f, 75.0f));

    void keyCallback(GLFWwindow* window, int key, int, int cmnd, int) {
        if(cmnd != GLFW_PRESS) {
            return;
        }

        if(key == GLFW_KEY_ESCAPE){
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    void framebufferSizeCallback(GLFWwindow*, int width, int height){
        viewportWidth = width;
        viewportHeight = height;
        glViewport(0, 0, width, height);
    }
}

int main(){
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    if(!glfwInit()){
        std::fprintf(stderr, "GLFW init error.\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    #endif

    GLFWwindow* window = glfwCreateWindow(viewportWidth, viewportHeight, "Project", nullptr, nullptr);

    if(!window){
        std::fprintf(stderr, "Window creation error.\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);

    if(!gladLoadGL(glfwGetProcAddress)){
        std::fprintf(stderr, "Error loading GL through GLAD.\n");
        return 1;
    }

    Sky sky;
    Shader skyShader("shaders/sky/sky.frag", "shaders/sky/sky.vert");
    Shader textShader("shaders/runtimeDiagnostics/frameRate.frag", "shaders/runtimeDiagnostics/frameRate.vert");
    RenderText hud;

    float fpsTotalTime = 0.0f;
    int fpsTotalFrames = 0;
    float prevFrame = 0.0f;
    float fpsResult = 0.0f;

    while(!glfwWindowShouldClose(window)){
        float currFrame = static_cast<float>(glfwGetTime());
        float frameTime = currFrame - prevFrame;
        prevFrame = currFrame;
        fpsTotalTime += frameTime;
        fpsTotalFrames++;

        if(fpsTotalTime >= 0.25f){
            fpsResult = static_cast<float>(fpsTotalFrames) / fpsTotalTime;
            fpsTotalFrames = 0;
            fpsTotalTime = 0.0f;
        }

        glClear(GL_COLOR_BUFFER_BIT);

        skyShader.use();
        sky.drawSky();

        textShader.use();

        char fpsBuff[32];
        std::snprintf(fpsBuff, sizeof(fpsBuff), "FPS:%.0f", fpsResult);

        float hudScale = 3.0f;
        const float hudMargin = 12.0f;
        const glm::vec3 hudColor(1.0f, 1.0f, 1.0f);

        float fpsDisplayWidth = RenderText::txtWidth(fpsBuff, hudScale);
        hud.drawTxt(
            textShader,
            fpsBuff,
            viewportWidth - hudMargin - fpsDisplayWidth,
            hudMargin,
            hudScale,
            hudColor,
            viewportWidth,
            viewportHeight
        );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}