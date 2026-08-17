#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "config.h"

#include "scene/user/camera/camera.h"
#include "scene/landscape/sky/sky.h"
#include "scene/landscape/clouds/seaOfClouds.h"
#include "scene/landscape/clouds/cloudLandscape.h"
#include "scene/landscape/props/props.h"
#include "scene/user/wyvern/wyvern.h"
#include "renders/frustum/frustum.h"
#include "renders/shader/shader.h"
#include "renders/shadow/shadow.h"
#include "renders/diagnostics/text.h"

#include <cstdio>
#include <cmath>


namespace {
    int viewportWidth = Config::windowWidth;
    int viewportHeight = Config::windowHeight;

    bool startingMousePos = true;
    bool wireframeDisplay = false;
    bool fullscreenEnabled = true;

    const int windowedWidth = Config::windowWidth;
    const int windowedHeight = Config::windowHeight;

    float mouseX = viewportWidth / 2.0f;
    float mouseY = viewportHeight / 2.0f;

    Camera camera(glm::vec3(0.0f, 24.0f, 75.0f));
    Wyvern wyvern(glm::vec3(0.0f, 24.0f, 75.0f));

    const float camHeight = Config::cameraHeight;
    const float camDist = Config::cameraDistance;

    const float lightSourceDist = 40.0f;
    const float shadowFarPlane = lightSourceDist + 80.0f;
    const float orthoProjectionSize = 10.0f;

    void keyCallback(GLFWwindow* window, int key, int, int cmnd, int) {
        if(cmnd != GLFW_PRESS) {
            return;
        }

        if(key == GLFW_KEY_ESCAPE){
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        if(key == GLFW_KEY_TAB){
            wireframeDisplay = !wireframeDisplay;
        }

        if(key == GLFW_KEY_F11){
            fullscreenEnabled = !fullscreenEnabled;
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);

            if(fullscreenEnabled){
                glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            }
            else{
                int xpos = (mode->width - windowedWidth) / 2;
                int ypos = (mode->height - windowedHeight) / 2;
                glfwSetWindowMonitor(window, nullptr, xpos, ypos, windowedWidth, windowedHeight, GLFW_DONT_CARE);
            }
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

        float x = static_cast<float>(xPos) - mouseX;
        mouseX = static_cast<float>(xPos);

        float y = static_cast<float>(yPos) - mouseY;
        mouseY = static_cast<float>(yPos);

        wyvern.processMouseInput(x, y);
    }
}

/*
    Main strings together all the classes and helper functions calling
    respective functions defined in the context or generated instance.
    To check functionality, a comment can be found above constructors,
    destructors or below the namespace, briefly summarizing the 
    role they play in the code base
*/
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

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    viewportWidth = videoMode->width;
    viewportHeight = videoMode->height;
    glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);

    GLFWwindow* window = glfwCreateWindow(viewportWidth, viewportHeight, "Project", primaryMonitor, nullptr);

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
    cloudParams.chunkSize = Config::cloudChunkSize;
    cloudParams.spawnRate = Config::cloudSpawnRate;
    const glm::vec3 cloudColor(0.9f, 0.9f, 0.95f);
    Shader cloudsShader("shaders/clouds/clouds.frag", "shaders/clouds/clouds.vert");
    CloudLandscape cloudLandscape(cloudParams);
    cloudLandscape.renderRadius = Config::cloudRenderRadius;
    cloudLandscape.derenderRadius = Config::cloudDerenderRadius;

    Shader propShader("shaders/props/props.frag", "shaders/props/props.vert");

    Prop lighthouseIsland(
        PropParams{
            55.0f, 90.0f,
            0.9f, 1.4f,
            1, 1,
            0.0f, Config::lighthouseLoadRadius,
            Config::lighthouseChunkSize,
            Config::lighthouseSpawnRate,
            7777
        },
        "assets/low_poly_lighthouse_island/scene.gltf"
    );

    Prop balloons(
        PropParams{
            22.0f, 46.0f, 
            3.0f, 4.5f,
            2, 4,
            18.0f, Config::balloonLoadRadius,
            Config::balloonChunkSize,
            Config::balloonSpawnRate,
            6666
        },
        "assets/low_poly_air_baloon/scene.gltf"
    );

    Prop toriiIsland(
        PropParams{
            18.0f, 44.0f,
            0.5f, 0.85f,
            1, 1,
            0.0f, Config::toriiLoadRadius,
            Config::toriiChunkSize,
            Config::toriiSpawnRate,
            5555
        },
        "assets/low_poly_torii_island/scene.gltf"
    );

    Prop treeIsland(
        PropParams{
            15.0f, 40.0f,
            5.0f, 8.0f,
            1, 1,
            0.0f, Config::treeLoadRadius,
            Config::treeChunkSize,
            Config::treeSpawnRate,
            4444
        },
        "assets/low_poly_tree_island/scene.gltf"
    );

    for(Prop* prop : {&lighthouseIsland, &balloons, &toriiIsland, &treeIsland}){
        if(!prop->load()){
            std::fprintf(stderr, "Could not load props\n");
            return 1;
        }
    }

    Shader wyvernShader("shaders/wyvern/wyvern.frag", "shaders/wyvern/wyvern.vert");
    if(!wyvern.loadWyvern("assets/wyvern/wyvern.glb")){
        std::fprintf(stderr, "Could not load wyvern model\n");
        return 1;
    }
    const glm::vec3 wyvernColor(0.55f, 0.32f, 0.30f);
    wyvern.speed = Config::wyvernSpeed;

    Shadow shadow(Config::shadowResolution);
    Shader shadowShader("shaders/shadow/shadow.frag", "shaders/shadow/shadow.vert");

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

        bool forward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        bool backward = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        bool left = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        bool right = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

        wyvern.processKeyInput(forward, backward, left, right, frameTime);
        camera.thirdPerson(
            camDist, 
            camHeight,
            frameTime,
            wyvern.pos,
            wyvern.forward,
            wyvern.up
        );

        glm::mat4 view = camera.viewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(Config::cameraFov),
            static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight),
            Config::cameraNearPlane, Config::cameraFarPlane
        );
        glm::mat4 viewProjectionInverted = glm::inverse(projection * view);
        glm::vec3 camPos = camera.position();
        Frustum frustum(projection * view);

        cloudLandscape.updateLandscape(camPos);
        for(Prop* prop : {&lighthouseIsland, &balloons, &toriiIsland, &treeIsland}){
            prop->update(camPos);
        }

        glm::vec3 lightPos = wyvern.pos + sunDirection * lightSourceDist;
        glm::vec3 lightUp;
        if(std::abs(glm::dot(sunDirection, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f){
            lightUp = glm::vec3(0.0f, 0.0f, 1.0f);
        }
        else{
            lightUp = glm::vec3(0.0f, 1.0f, 0.0f);
        }
        glm::mat4 lightView = glm::lookAt(lightPos, wyvern.pos, lightUp);
        glm::mat4 lightProjection = glm::ortho(
            -orthoProjectionSize, 
            orthoProjectionSize,
            -orthoProjectionSize,
            orthoProjectionSize,
            1.0f,
            shadowFarPlane
        );

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        shadow.start();
        shadowShader.use();
        shadowShader.uniSet("lightViewProjection", lightProjection * lightView);
        wyvern.drawShadow(shadowShader);
        shadow.stop(viewportWidth, viewportHeight);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(wireframeDisplay){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else{
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        skyShader.use();
        skyShader.uniSet("sunColor", sunColor);
        skyShader.uniSet("sunDirection", sunDirection);
        skyShader.uniSet("lowerSkyColor", lowerSkyColor);
        skyShader.uniSet("upperSkyColor", upperSkyColor);
        skyShader.uniSet("viewProjectionInverted", viewProjectionInverted);
        skyShader.uniSet("cameraPosition", camPos);
        sky.drawSky();

        shadow.bind(0);
        cloudsShader.use();
        cloudsShader.uniSet("viewProjection", projection * view);
        cloudsShader.uniSet("color", cloudColor);
        cloudsShader.uniSet("sunColor", sunColor);
        cloudsShader.uniSet("sunDirection", sunDirection);
        cloudsShader.uniSet("lightViewProjection", lightProjection * lightView);
        cloudsShader.uniSet("shadowMapping", 0);
        cloudLandscape.drawLandscape(frustum);

        shadow.bind(0);
        propShader.use();
        propShader.uniSet("viewProjection", projection * view);
        propShader.uniSet("sunColor", sunColor);
        propShader.uniSet("sunDirection", sunDirection);
        propShader.uniSet("lightViewProjection", lightProjection * lightView);
        propShader.uniSet("shadowMapping", 0);
        lighthouseIsland.drawProp(propShader);
        balloons.drawProp(propShader);
        toriiIsland.drawProp(propShader);
        treeIsland.drawProp(propShader);

        wyvernShader.use();
        wyvernShader.uniSet("viewProjection", projection * view);
        wyvernShader.uniSet("sunColor", sunColor);
        wyvernShader.uniSet("sunDirection", sunDirection);
        wyvern.drawWyvern(wyvernShader, wyvernColor);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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