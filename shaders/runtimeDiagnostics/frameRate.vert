#version 330 core 

layout(location = 0) in vec2 aPosition;

uniform vec2 uViewportSize;

void main() {
    vec2 normal = aPosition / uViewportSize;

    float xCoord = normal.x * 2.0 - 1.0;
    float yCoord = 1.0 - normal.y * 2.0;
    vec2 coord = vec2(xCoord, yCoord);

    gl_Position = vec4(coord, 0.0, 1.0);
}