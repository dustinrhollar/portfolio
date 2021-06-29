#version 330 core

struct Material {
    sampler2D diffuse;
	//sampler2D normal;
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

#define MAX_POINT_LIGHTS 10
uniform PointLight point_lights[MAX_POINT_LIGHTS];
uniform int        point_light_count;

uniform vec3 viewPos;
uniform Material material;


vec3 CalculateDirectionalLighting(DirectionalLight light, Material material, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), 100);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));  
  
    // combine results
    vec3 ambient  = light.ambient         * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

void main()
{
	// properties
    vec3 norm = normalize(inNormal);
    vec3 viewDir = normalize(viewPos - FragPos);

	// Directional Light
	vec3 result = CalculateDirectionalLighting(directional_light, material, norm, viewDir);
	
	// Point lights
	for (int i = 0; i < point_light_count; ++i)
	{
		result += CalculatePointLight(point_lights[i], norm, FragPos, viewDir);
	}

	// for loop for spot lights

	FragColor = vec4( result, 1.0 );
}