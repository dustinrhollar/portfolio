#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 textureCoordinate;

// View matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// texture sampler
uniform sampler2D heightTexture;
uniform sampler2D normalTexture;

// Scale
uniform float scale;

out vec4 TexCoord;
out vec3 FragPos;
out vec3 Normal;

void main()
{
    vec4 worldPos = model * vec4(aPos.x * 5, scale * texture( heightTexture, textureCoordinate.rg ).r * 5, aPos.y * 5,  1.0f);
    FragPos = worldPos.xyz;
	gl_Position = projection * view * worldPos;
	//gl_Position = vec4(aPos, 1.0f);

	mat3 normalMatrix = transpose(inverse(mat3(model)));
	Normal = normalMatrix * ( scale * texture( normalTexture, textureCoordinate.rg ).rgb );

	TexCoord = textureCoordinate;
}