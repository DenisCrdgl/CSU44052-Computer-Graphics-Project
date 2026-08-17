#include "scene/user/wyvern/wyvern.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

/*
    Summary: This object is responsible for the
    wyvern the player moves around. Uses processMouseInput()
    and processKeyInput() to register the movement the user
    wants with mouse movement representing looking direction
    and key pressing representing forward, backward, left and
    right. The orientation of the wyvern is updated
    by updateVec(). The model itself is built using modelMtrx()
    which takes its relevant positions and orientation and is then
    drawn using drawWyvern(). The function drawShadow() is used to
    pass the model through the depth shadow function in order for it 
    to cast shadows over any cloud/prop underneath
*/
Wyvern::Wyvern(const glm::vec3& position) : pos(position){
}

bool Wyvern::loadWyvern(const std::string& path){
    return model.loadWyvern(path);
}

glm::mat4 Wyvern::modelMtrx() const{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), pos);
    glm::mat4 rotation(1.0f);
    rotation[0] = glm::vec4(-right, 0.0f);
    rotation[1] = glm::vec4(up, 0.0f);
    rotation[2] = glm::vec4(forward, 0.0f);
    return translation * rotation;
}

void Wyvern::processMouseInput(float x, float y){
    yaw += x * sensitivity;
    pitch -= y * sensitivity;
    pitch = std::clamp(pitch, -maxPitch, maxPitch);
}

void Wyvern::updateVec(){
    float yawRadian = glm::radians(yaw);
    float pitchRadian = glm::radians(pitch);
    glm::vec3 temp;
    temp.x = std::cos(yawRadian) * std::cos(pitchRadian);
    temp.y = std::sin(pitchRadian);
    temp.z = std::sin(yawRadian) * std::cos(pitchRadian);
    
    forward = glm::normalize(temp);
    right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    up = glm::normalize(glm::cross(right, forward));
}

void Wyvern::processKeyInput(bool wKey, bool sKey, bool aKey, bool dKey, float frameTime){
    updateVec();
    float velocity = speed * frameTime;
    if(wKey){
        pos += forward * velocity;
    }
    if(sKey){
        pos -= forward * velocity;
    }
    if(aKey){
        pos -= right * velocity;
    }
    if(dKey){
        pos += right * velocity;
    }

    bool isMoving = wKey || sKey || aKey || dKey;
    std::string flightState;
    if(isMoving){
        flightState = "Flight";
    }
    else{
        flightState = "Stationary_Flight";
    }

    if(flightState != flightAnimation){
        flightAnimation = flightState;
        animationTime = 0.0f;
    }
    else{
        animationTime += frameTime;
    }
}

void Wyvern::drawWyvern(const Shader& shader, const glm::vec3& color) const{
    std::vector<glm::mat4> joints = model.jointMatrices(animationTime, flightAnimation);
    shader.uniSet("model", modelMtrx());
    shader.uniSet("jointsList", joints);
    shader.uniSet("base", color);
    model.drawWyvern();
}

void Wyvern::drawShadow(const Shader& shader) const{
    std::vector<glm::mat4> joints = model.jointMatrices(animationTime, flightAnimation);
     shader.uniSet("model", modelMtrx());
    shader.uniSet("jointsList", joints);
    model.drawWyvern();
}