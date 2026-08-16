#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 inputNormal;
layout(location = 2) in vec3 inputColor;
layout(location = 3) in vec4 weights;
layout(location = 4) in vec4 joints;

uniform mat4 model;
uniform mat4 jointsList[40];
uniform mat4 viewProjection;

out vec3 normal;
out vec3 color;
out vec3 worldPos;

void main(){
    mat4 skin =
        weights.x * jointsList[int(joints.x)] +
        weights.y * jointsList[int(joints.y)] +
        weights.z * jointsList[int(joints.z)] +
        weights.w * jointsList[int(joints.w)];

    vec4 skinPosition = skin * vec4(position, 1.0);
    gl_Position = viewProjection * model * skinPosition;

    color = inputColor;
    normal = mat3(model) * mat3(skin) * inputNormal;
    worldPos = (model * skinPosition).xyz;
}