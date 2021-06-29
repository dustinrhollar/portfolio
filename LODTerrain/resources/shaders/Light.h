
typedef struct {
    sampler2D diffuse;
    sampler2D specular;    
    float shininess;
} Material;

typedef struct
{
    vec3 direction;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} DirectionalLight;

typedef struct
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
    vec3 position;
} PointLight;

vec3 CalculateDirectionalLighting(DirLight light, vec3 color, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 17);
    // combine results
    vec3 ambient = light.ambient; // * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = color * light.diffuse * diff; // * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec; // * vec3(texture(material.specular, TexCoords));
    return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, vec3 color, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 17);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient;// * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = color * light.diffuse * diff; // * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec; // * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
