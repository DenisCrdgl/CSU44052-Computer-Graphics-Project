#version 330 core 

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 inputNormal;
layout(location = 2) in vec3 inputColor;

out vec3 normal;
out vec3 color;

uniform mat4 model;
uniform mat4 viewProjection;

void main(){
    gl_Position =  viewProjection * model * vec4(position, 1.0);
    normal = mat3(model) * inputNormal;
    color = inputColor;
}