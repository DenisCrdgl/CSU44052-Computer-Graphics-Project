#version 330 core

in vec2 NDC;

out vec4 skyColor;

uniform vec3 sunColor;
uniform vec3 sunDirection;

uniform vec3 lowerSkyColor;
uniform vec3 upperSkyColor;

uniform mat4 viewProjectionInverted;
uniform vec3 cameraPosition;

void main() {
    vec4 worldView = viewProjectionInverted * vec4(NDC, 1.0, 1.0);
    vec3 worldPerspective = worldView.xyz / worldView.w;
    vec3 viewDirection = normalize(worldPerspective - cameraPosition);

    float colorDiffusion = clamp(viewDirection.y * 0.5 + 0.5, 0.0, 1.0);
    colorDiffusion = pow(colorDiffusion, 0.55);
    vec3 colorResult = mix(lowerSkyColor, upperSkyColor, colorDiffusion);

    float sunStrength = max(dot(viewDirection, normalize(sunDirection)), 0.0);
    colorResult += sunColor * pow(sunStrength, 400.0) * 2.0;
    colorResult += sunColor * pow(sunStrength, 8.0) * 0.15;

    skyColor = vec4(colorResult, 1.0);

}
