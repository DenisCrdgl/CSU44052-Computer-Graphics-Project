#version 330 core 

in vec3 normal;
in vec3 color;
in vec3 worldPos;

out vec4 propColor;

uniform vec3 sunDirection;
uniform vec3 sunColor;
uniform mat4 lightViewProjection;
uniform sampler2D shadowMapping;

float shadowSampling(vec3 worldPos, float lightNormal){
    vec4 space = lightViewProjection * vec4(worldPos, 1.0);
    vec3 projection = space.xyz / space.w * 0.5 + 0.5;
    if(projection.z > 1.0){
        return 0.0;
    }
    float bias = max(0.005 * (1.0 - lightNormal), 0.0015);
    
    if(projection.z - bias > texture(shadowMapping, projection.xy).r){
        return 1.0;
    }
    else{
        return 0.0;
    }
}

void main(){
    vec3 norm = normalize(normal);
    float ambience = 0.5 + norm.y * 0.5;

    float diffusion = 0.5;
    float lightNormal = clamp(
        (dot(norm, sunDirection) + diffusion) / (1.0 + diffusion),
        0.0,
        1.0
    );
    float sunStrength = 1.0 - shadowSampling(worldPos, lightNormal) * 0.7;
    vec3 brightness = mix(0.6, 1.0, ambience) + 
        sunColor * lightNormal * sunStrength;
    propColor = vec4(color * brightness, 1.0);
}