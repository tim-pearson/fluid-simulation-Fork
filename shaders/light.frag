#version 450 core

in vec3 fragColor;

out vec4 FragColor;

void main() {
    vec2 uv = gl_PointCoord * 2.0 - 1.0;
    if (dot(uv, uv) > 1.0) discard;      // circular mask
    FragColor = vec4(fragColor, 1.0);
}