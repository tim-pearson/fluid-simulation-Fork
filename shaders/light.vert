#version 450 core

uniform vec3 inPos;
uniform vec3 inColor;

out vec3 fragColor;

uniform mat4 viewProj;

void main() {
    gl_Position = viewProj * vec4(inPos, 1.0);
    gl_PointSize = 16.0; // radius of marker
    fragColor = inColor;
}