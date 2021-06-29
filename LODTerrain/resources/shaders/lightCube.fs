#version 330 core

struct Material {
    sampler2D diffuse;
	sampler2D normal;
    sampler2D specular;    
    float shininess;
};

struct DirectionalLight
{
    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
    vec3 position;
};

in vec3 FragPos;
in vec2 TexCoords;
in vec3 inColor;
in vec3 inNormal;

out vec4 FragColor;

uniform DirectionalLight directional_light;
uniform vec3 viewPos;
uniform Material material;

void main()
{
	FragColor = vec4(inColor, 1.0);
}