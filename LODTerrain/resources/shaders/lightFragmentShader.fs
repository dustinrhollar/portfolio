#version 330 core

struct Light {
    vec3 Position;
    vec3 Color;

    float Linear;
    float Quadratic;
    float Radius;
};

struct DirectionalLight {
    vec3 Direction;

    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
};

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

//const int NR_LIGHTS = 32;
//uniform Light light;
uniform DirectionalLight light;
uniform vec3 viewPos;

void pointLight()
{
/*
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);

    for( int i = 0; i < 30; i++) {
        // diffuse
        vec3 lightDir = normalize(light.Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.Color;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = light.Color * spec * Specular;
        // attenuation
        float distance = length(light.Position - FragPos);
        float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }
*/
}

vec3 directionalLight( DirectionalLight light, vec3 normal, vec3 viewDir )
{
    vec3 lightDir = normalize(-light.Direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), texture(gAlbedoSpec, TexCoords).a);
    // combine results
    vec3 ambient  = light.Ambient  * vec3(texture(gAlbedoSpec, TexCoords));
    vec3 diffuse  = light.Diffuse  * diff * vec3(texture(gAlbedoSpec, TexCoords));
    vec3 specular = light.Specular * spec * vec3(texture(gAlbedoSpec, TexCoords));
    return (ambient + diffuse + specular);
}

void main()
{
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 viewdir = normalize(FragPos - viewPos);
    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb);

    FragColor = vec4(directionalLight( light, Normal, viewdir ), 1.0);
}