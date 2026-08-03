#version 330 core 

out vec4 TxtColor;

uniform vec3 uColor;

void main() {
    TxtColor = vec4(uColor, 1.0);
}