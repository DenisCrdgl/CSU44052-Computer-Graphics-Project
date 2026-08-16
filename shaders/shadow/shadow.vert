#version 330 core

layout(location = 0) in vec3 position;
layout(location = 3) in vec4 weights;
layout(location = 4) in vec4 joints;

uniform mat4 jointsList[40];
uniform mat4 model;
uniform mat4 lightViewProjection;

void main(){
    mat4 skin =
        weights.x * jointsList[int(joints.x)] + 
        weights.y * jointsList[int(joints.y)] +
        weights.z * jointsList[int(joints.z)] +
        weights.w * jointsList[int(joints.w)];

    vec4 skinPosition = skin * vec4(position, 1.0);
    gl_Position = lightViewProjection * model * skinPosition;
}