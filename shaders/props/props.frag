#version 330 core 

in vec3 normal;
in vec3 color;

out vec4 propColor;

void main(){
    vec3 vertNormal = normalize(normal);
    float ambience = 0.5 + vertNormal.y * 0.5;
    propColor = vec4(color * mix(0.6, 1.0, ambience), 1.0);
}