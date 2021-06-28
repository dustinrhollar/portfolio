
void render_component_init(RenderComponent *comp, b8 indices, DrawMode mode)
{
    glGenVertexArrays(1, &comp->vao);
    glGenBuffers(1, &comp->vbo);
    if (indices) glGenBuffers(1, &comp->ebo);
    comp->has_indices = indices;
    comp->mode = mode;
    comp->element_count = 0;
    comp->offset = 0;
}

void render_component_free(RenderComponent *comp)
{
    glDeleteVertexArrays(1, &comp->vao);
    glDeleteBuffers(1, &comp->vbo);
    if (comp->has_indices) glDeleteBuffers(1, &comp->ebo);
    comp->element_count = 0;
    comp->offset = 0;
}

void render_component_draw(RenderComponent *comp)
{
    glBindVertexArray(comp->vao);
    if (comp->has_indices)
    {
        glDrawElements(comp->mode, comp->element_count, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(comp->mode, comp->offset, comp->element_count);
    }
    glBindVertexArray(0);
}

void renderer_init(Renderer *renderer)
{
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, (GLint*)&renderer->ubo_min_alignment);

    // Global Data

    renderer->ubo_gd_size = 2 * sizeof(ShaderGlobalData);
    renderer->ubo_gd_size = memory_align(renderer->ubo_gd_size, renderer->ubo_min_alignment);

    glGenBuffers(1, &renderer->global_data);
    glBindBuffer(GL_UNIFORM_BUFFER, renderer->global_data);
    glBufferData(GL_UNIFORM_BUFFER, renderer->ubo_gd_size, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Lighting Data
    
    renderer->ubo_lt_size = sizeof(ShaderLightingData);
    renderer->ubo_lt_size = memory_align(renderer->ubo_lt_size, renderer->ubo_min_alignment);

    glGenBuffers(1, &renderer->light_data);
    glBindBuffer(GL_UNIFORM_BUFFER, renderer->light_data);
    glBufferData(GL_UNIFORM_BUFFER, renderer->ubo_lt_size, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void renderer_free(Renderer *renderer)
{
    glDeleteBuffers(1, &renderer->global_data);
    glDeleteBuffers(1, &renderer->light_data);
}

void renderer_prepare_shader_uniforms(ShaderProgram *shader)
{
    shader_program_set_uniform_block(shader, "GlobalData", SHADER_UBO_BINDING_GLOBAL);
    shader_program_set_uniform_block(shader, "LightData",  SHADER_UBO_BINDING_LIGHTING);
}

