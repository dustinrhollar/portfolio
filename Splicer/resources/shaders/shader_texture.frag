#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 OutColor;
layout (location = 1) in vec3 OutNormal;
layout (location = 2) in vec2 OutTex;

layout (location = 0) out vec4 FragColor;

layout(binding = 0, set=2) uniform sampler2D texSampler;

void main()
{
    FragColor = texture(texSampler, OutTex);
}