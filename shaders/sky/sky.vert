#version 330 core

out vec2 NDC;

layout(location = 0) in vec2 position;

void main() {
    NDC = position;
    gl_Position = vec4(position, 1.0, 1.0);
}