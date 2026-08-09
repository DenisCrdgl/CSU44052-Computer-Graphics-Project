#version 330 core 

out vec4 txtColor;

uniform vec3 color;

void main() {
    txtColor = vec4(color, 1.0);
}