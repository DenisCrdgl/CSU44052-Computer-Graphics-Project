#version 330 core

in vec3 normal;
in vec3 color;

uniform vec3 base;

out vec4 wyvernColor;

void main(){
    vec3 vertNormal = normalize(normal);
    float ambience = 0.5 * vertNormal.y + 0.5;
    wyvernColor = vec4(color * base * mix(0.6, 1.0, ambience), 1.0);
}