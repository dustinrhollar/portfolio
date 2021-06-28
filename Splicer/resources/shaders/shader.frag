#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 OutColor;
layout (location = 1) in vec3 OutNormal;
layout (location = 2) in vec2 OutTex;

layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = vec4(OutColor, 1.0f);
}