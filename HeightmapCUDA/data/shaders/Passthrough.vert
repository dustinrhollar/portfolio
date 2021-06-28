#version 430 core

layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 Texcoords;

out vec2 v_Texcoords;

void main(void)
{
	v_Texcoords = Texcoords;
	gl_Position = vec4(Position, 0.0, 1.0);
}