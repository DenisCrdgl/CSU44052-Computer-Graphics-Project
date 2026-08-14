#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renders/shader/shader.h"
#include "renders/diagnostics/text.h"

#include "scene/user/camera/camera.h"
#include "scene/landscape/sky/sky.h"
#include "scene/landscape/clouds/seaOfClouds.h"
#include "scene/landscape/clouds/cloudLandscape.h"
#include "renders/frustum/frustum.h"

#include <cstdio>


namespace {
    int viewportWidth = 1280;
    int viewportHeight = 720;

    bool startingMousePos = true;

    float mouseX = viewportWidth / 2.0f;
    float mouseY = viewportHeight / 2.0f;

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

    void cursorCallback(GLFWwindow*, double xPos, double yPos){
        if(startingMousePos){
            mouseX = static_cast<float>(xPos);
            mouseY = static_cast<float>(yPos);
            startingMousePos = false;
        }

        float xRate = static_cast<float>(xPos) - mouseX;
        mouseX = static_cast<float>(xPos);

        float yRate = static_cast<float>(yPos) - mouseY;
        mouseY = static_cast<float>(yPos);

        camera.processMouse(xRate, yRate);
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
    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if(!gladLoadGL(glfwGetProcAddress)){
        std::fprintf(stderr, "Error loading GL through GLAD.\n");
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    Sky sky;
    const glm::vec3 sunColor(1.0f, 0.96f, 0.88f);
    const glm::vec3 sunDirection = glm::normalize(glm::vec3(-0.45f, 0.65f, -0.35f));
    const glm::vec3 lowerSkyColor(0.75f, 0.82f, 0.92f);
    const glm::vec3 upperSkyColor(0.20f, 0.42f, 0.80f);
    Shader skyShader("shaders/sky/sky.frag", "shaders/sky/sky.vert");

    RenderText hud;
    Shader textShader("shaders/runtimeDiagnostics/runtimeDiagnostics.frag", "shaders/runtimeDiagnostics/runtimeDiagnostics.vert");

    CloudSeaParams cloudParams;
    const glm::vec3 cloudColor(0.9f, 0.9f, 0.95f);
    Shader cloudsShader("shaders/clouds/clouds.frag", "shaders/clouds/clouds.vert");
    CloudLandscape cloudLandscape(cloudParams);

    int fpsTotalFrames = 0;

    float fpsTotalTime = 0.0f;
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

        bool up = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        bool down = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
        bool left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        bool right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
        bool forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        bool backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;

        camera.processKeys(up, down, left, right, forward, backward, frameTime);

        glm::mat4 view = camera.viewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(65.0f),
            static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight),
            0.1f, 1000.0f
        );
        glm::mat4 viewProjectionInverted = glm::inverse(projection * view);
        glm::vec3 camPos = camera.position();
        Frustum frustum(projection * view);

        cloudLandscape.updateLandscape(camPos);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skyShader.use();
        skyShader.uniSet("sunColor", sunColor);
        skyShader.uniSet("sunDirection", sunDirection);
        skyShader.uniSet("lowerSkyColor", lowerSkyColor);
        skyShader.uniSet("upperSkyColor", upperSkyColor);
        skyShader.uniSet("viewProjectionInverted", viewProjectionInverted);
        skyShader.uniSet("cameraPosition", camPos);
        sky.drawSky();

        cloudsShader.use();
        cloudsShader.uniSet("viewProjection", projection * view);
        cloudsShader.uniSet("color", cloudColor);
        cloudLandscape.drawLandscape(frustum);

        glDisable(GL_DEPTH_TEST);
        textShader.use();

        float hudScale = 3.0f;
        const float hudMargin = 12.0f;
        const glm::vec3 hudColor(1.0f, 1.0f, 1.0f);
        float lineHeight = 7.0f * hudScale + 2.0f * hudScale;

        char fpsBuff[32];
        std::snprintf(
            fpsBuff, 
            sizeof(fpsBuff), 
            "FPS:%.0f", 
            fpsResult
        );
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

        char coordBuff[64];
        std::snprintf(
            coordBuff, 
            sizeof(coordBuff), 
            "X:%.0f, Y:%.0f, Z:%.0f", 
            camPos.x, camPos.y, camPos.z
        );
        float coordDisplayWidth = RenderText::txtWidth(coordBuff, hudScale);
        hud.drawTxt(
            textShader,
            coordBuff,
            viewportWidth - hudMargin - coordDisplayWidth,
            hudMargin + lineHeight,
            hudScale,
            hudColor,
            viewportWidth,
            viewportHeight
        );

        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}