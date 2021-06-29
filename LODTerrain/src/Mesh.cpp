
// TODO(Dustin): Default Pipeline?
void CreateMesh(Mesh *mesh, int pipelineId, bool hasEBO)
{
    mesh->pipelineId = pipelineId;
    mesh->drawMode = DRAW_MODE_TRIANGLE_ELEMENT;
    
    // buffers
    mesh->hasEBO = hasEBO;
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    if (mesh->hasEBO)
    {
        glGenBuffers(1, &mesh->EBO);
    }
    
    // Number of elements in the EBO or VBO
    mesh->size = 0;
    
    mesh->model = glm::mat4(1.0);
    mesh->proj  = glm::mat4(1.0);
}

void FreeMesh(Mesh *mesh)
{
    // Free VAO and VBO
    glDeleteVertexArrays(1, &mesh->VAO);
    glDeleteBuffers(1, &mesh->VBO);
    
    if (mesh->hasEBO)
    {
        glDeleteBuffers(1, &mesh->EBO);
    }
}

void CreateMeshList(MeshList *list, int cap)
{
    list->size = 0;
    list->cap  = (cap < 0) ? 0 : cap;
    list->meshes = (Mesh *)malloc(sizeof(Mesh) * cap);
}

void AddMesh(MeshList *list, Mesh *mesh)
{
    if (!list || !mesh)
        return;
    
    if (list->size + 1 >= list->cap)
    {
        list->cap *= 2;
        list->meshes = (Mesh *)realloc(list->meshes, list->cap);
    }
    
    memcpy(&list->meshes[list->size], mesh, sizeof(Mesh));
    ++list->size;
}

void FreeMeshList(MeshList *list)
{
    for (int i = 0; i < list->size; ++i)
    {
        FreeMesh(&list->meshes[i]);
    }
    
    free(list->meshes);
    list->size = 0;
    list->cap  = 0;
}

void RenderMesh(Mesh *mesh, Light *dirLight, Light *list, Camera* camera)
{
    Pipeline *pipeline = GetPipeline(mesh->pipelineId); 
    // &GlobalPipelineManager->pipes[mesh->pipelineId];
    Shader *shader = pipeline->geometryShader;
    shader->use();
    
    // setup lighting
    int point_count = 0;
    int dir_count = 0;
    
    // Set the directional light
    shader->setVec3("directional_light.ambient",  dirLight->ambient);
    shader->setVec3("directional_light.diffuse",  dirLight->diffuse);
    shader->setVec3("directional_light.specular", dirLight->specular);
    
    DirectionalLight *dl = AS_DIRECTIONAL_LIGHT(dirLight);
    shader->setVec3("directional_light.direction", dl->direction);
    
    ++dir_count;
    
    // For now, the only other type of light is a Point light
    // Set point light count
    shader->setInt("point_light_count", arrlen(list));
    
    char buffer[64];
    for (int i = 0; i < arrlen(list); ++i)
    {
        Light *light = &list[i];
        
        switch(light->type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
            { // There is only one directional light
            } break;
            case LIGHT_TYPE_POINT:
            {
                sprintf(buffer, "point_lights[%i].ambient", point_count);
                shader->setVec3(buffer, light->ambient);
                
                sprintf(buffer, "point_lights[%i].diffuse", point_count);
                shader->setVec3(buffer, light->diffuse);
                
                sprintf(buffer, "point_lights[%i].specular", point_count);
                shader->setVec3(buffer, light->specular);
                
                PointLight *pl = AS_POINT_LIGHT(light);
                
                sprintf(buffer, "point_lights[%i].position", point_count);
                shader->setVec3(buffer, pl->position);
                
                sprintf(buffer, "point_lights[%i].quadratic", point_count);
                shader->setFloat(buffer, pl->quadratic);
                
                sprintf(buffer, "point_lights[%i].linear", point_count);
                shader->setFloat(buffer, pl->linear);
                
                sprintf(buffer, "point_lights[%i].constant", point_count);
                shader->setFloat(buffer, pl->constant);
                
                ++point_count;
            } break;
            default: break;
        }
    }
    
    // bind the textures
    if (pipeline->hasDiffuse)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pipeline->diffuseTexture.id);
    }
    
    if (pipeline->hasNormal)
    { // Not yet
        //shader->setInt("material.normal", 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pipeline->normalTexture.id);
    }
    
    if (pipeline->hasAlbedo)
    {
        //shader->setInt("material.albedo", 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, pipeline->albedoTexture.id);
    }
    
    shader->setFloat("material.shininess", pipeline->shininess);
    
    // draw
    glBindVertexArray(mesh->VAO);
    
    shader->setMat4("model",      mesh->model);
    shader->setMat4("projection", mesh->proj);
    shader->setMat4("view",       camera->GetViewMatrix());
    
    shader->setVec3("viewPos", camera->Position);
    
    switch (mesh->drawMode)
    {
        case DRAW_MODE_TRIANGLE:
        {
            glDrawArrays(GL_TRIANGLES, 0, mesh->size);
        } break;
        case DRAW_MODE_TRIANGLE_ELEMENT:
        {
            glDrawElements(GL_TRIANGLES,
                           mesh->size, 
                           GL_UNSIGNED_INT, 0);
        } break;
        case DRAW_MODE_TRIANGLE_STRIP:
        {
        } break;
        case DRAW_MODE_TRIANGLE_STRIP_ELEMENT:
        {
        } break;
        // Instanced version
        case DRAW_MODE_INSTANCED_TRIANGLE:
        {
        } break;
        case DRAW_MODE_INSTANCED_TRIANGLE_ELEMENT:
        {
        } break;
        case DRAW_MODE_INSTANCED_TRIANGLE_STRIP:
        {
        } break;
        case DRAW_MODE_INSTANCED_TRIANGLE_STRIP_ELEMENT:
        {
        } break;
        default: break;
    }
}