#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 textureCoordinate;

out vec2 TexCoords;

void main()
{
    TexCoords = textureCoordinate.rg;
    gl_Position = vec4(aPos, 1.0);
}