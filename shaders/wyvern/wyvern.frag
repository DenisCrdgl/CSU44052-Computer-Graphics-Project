#version 330 core

in vec3 normal;
in vec3 color;
in vec3 worldPos;

out vec4 wyvernColor;

uniform vec3 base;
uniform vec3 sunDirection;
uniform vec3 sunColor;

void main(){
    vec3 norm = normalize(normal);
    float ambience = 0.5 * norm.y + 0.5;

    float diffusion = 0.5;
    float lightNormal = clamp(
        (dot(norm, sunDirection) + diffusion) / (1.0 + diffusion),
        0.0,
        1.0
    );
    vec3 brightness = mix(0.6, 1.0, ambience) + 
        sunColor * lightNormal;
    wyvernColor = vec4(color * base * brightness, 1.0);
}