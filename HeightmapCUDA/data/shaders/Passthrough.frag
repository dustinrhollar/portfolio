#version 430 core

in vec2 v_Texcoords;
uniform sampler2D u_image;

out vec4 FragColor;

void main(void)
{
	FragColor = texture2D(u_image, v_Texcoords);
}