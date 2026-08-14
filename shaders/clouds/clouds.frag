#version 330 core

in vec3 vertNormal;
out vec4 cloudColor;

uniform vec3 color;

void main(){
    vec3 norm = normalize(vertNormal);
    float ambience = norm.y * 0.5 + 0.5;
    cloudColor = vec4(color * mix(0.6, 1.0, ambience), 1.0);
}