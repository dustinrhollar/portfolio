
#ifndef LIGHTING_H
#define LIGHTING_H

typedef struct {
    //sampler2D diffuse;
    //sampler2D specular;    
    float shininess;
} Material;

typedef enum
{
    LIGHT_TYPE_DIRECTIONAL,
    LIGHT_TYPE_POINT,
} LightType;

typedef struct
{
    glm::vec3 direction;
} DirectionalLight;

typedef struct {
    float constant;
    float linear;
    float quadratic;
    glm::vec3 position;  
} PointLight; 

typedef struct
{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
	
    LightType type;
    
    union
    {
        DirectionalLight dl;
        PointLight       pl;
    };
} Light;

#define IS_DIRECTIONAL_LIGHT(light) ((Light*)light)->type == LIGHT_TYPE_DIRECTIONAL
#define IS_POINT_LIGHT(light)       ((Light*)light)->type == LIGHT_TYPE_POINT 

#define AS_DIRECTIONAL_LIGHT(light) &(((Light*)light)->dl)
#define AS_POINT_LIGHT(light)       &(((Light*)light)->pl)

typedef struct 
{
    int size      = 0;
    int cap       = 0; 
    Light *lights = NULL;
} LightList;

void CreateDirectionalLight(Light *light, glm::vec3 dir, 
                            glm::vec3 ambientColor,
                            glm::vec3 diffuseColor,
                            glm::vec3 specularColor);

void CreatePointLight(Light *light, 
                      glm::vec3 ambientColor,
                      glm::vec3 diffuseColor,
                      glm::vec3 specularColor,
                      glm::vec3 position,
                      float constant,
                      float linear,
                      float quadratic);

void CreateLightList(LightList *list, int cap);
void AddLightToLightList(LightList *list, Light *light);
void FreeLightList(LightList *list);

#endif // LIGHTING_H