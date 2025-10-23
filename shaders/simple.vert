#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in vec3 aInstancePos;  // Per-instance position
layout(location = 4) in float aInstanceColor; // Per-instance color value

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out float ParticleColor;

void main()
{
    // hardcoded radius
    vec3 worldPos = aPos * 0.05 + aInstancePos;
    
    FragPos = worldPos;
    Normal = aNormal;
    ParticleColor = aInstanceColor;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}