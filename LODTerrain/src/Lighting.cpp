
void CreateDirectionalLight(Light *light, glm::vec3 dir, 
                            glm::vec3 ambientColor,
                            glm::vec3 diffuseColor,
                            glm::vec3 specularColor)
{
    light->ambient   = ambientColor;
    light->diffuse   = diffuseColor;
    light->specular  = specularColor;
    light->type      = LIGHT_TYPE_DIRECTIONAL;
    light->dl.direction = dir;
}

void CreatePointLight(Light *light, 
                      glm::vec3 ambientColor,
                      glm::vec3 diffuseColor,
                      glm::vec3 specularColor,
                      glm::vec3 position,
                      float constant,
                      float linear,
                      float quadratic)
{
    light->ambient   = ambientColor;
    light->diffuse   = diffuseColor;
    light->specular  = specularColor;
    light->type      = LIGHT_TYPE_POINT;
    light->pl.position  = position;  
    light->pl.constant  = constant;
    light->pl.linear    = linear;
    light->pl.quadratic = quadratic;
}

#define DEFAULT_LIGHT_LIST_SIZE 10
void CreateLightList(LightList *list, int cap)
{
    if (list->cap != 0)
    {
        FreeLightList(list);
    }
    
    list->size = 0;
    list->cap  = (cap < 0) ? DEFAULT_LIGHT_LIST_SIZE : cap;
    list->lights = (Light*)malloc(sizeof(Light) * list->cap);
}

void AddLightToLightList(LightList *list, Light *light)
{
    if (list->size + 1 >= list->cap )
    {
        list->cap *= 2;
        list->lights = (Light*)realloc(list->lights, list->cap * sizeof(Light));
    }
    
    memcpy(&list->lights[list->size], light, sizeof(Light));
    ++list->size;
}

void FreeLightList(LightList *list)
{
    free(list->lights);
    list->size = 0;
    list->cap  = 0;
}
