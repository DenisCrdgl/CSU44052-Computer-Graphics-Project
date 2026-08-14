#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 inputNormal;

uniform mat4 viewProjection;

out vec3 vertNormal;

void main(){
    gl_Position = viewProjection * vec4(position, 1.0);
    vertNormal = inputNormal;
}