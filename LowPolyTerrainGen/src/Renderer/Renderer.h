
#ifndef _RENDERER_H
#define _RENDERER_H

typedef u32    VertexArray;
typedef u32    VertexBuffer;
typedef u32    ElementBuffer;
typedef GLenum DrawMode;

typedef struct
{
    VertexArray   vao;
    VertexBuffer  vbo;
    ElementBuffer ebo;
    b8            has_indices;
    // if has indices == true, this is index count, otherwise vertex count
    u32           element_count; 
    u32           offset; // buffer offset
    DrawMode      mode;
} RenderComponent;

// Global Data is a UBO that stores lighting and camera information.
// Now, the camera has two ways to be rendered: 
// 1. The exact position of the camera
// 2. The inverted position of the camera
//
// Version 2 is used for rendering reflections into a FBO. In order,
// to account for this, the size of this UBO is 2 * sizeof(ShaderGlobalSata)
//
// A user will have to bind the buffer range two times in a frame:
// 1. Offset sizeof(ShaderGlobalState) -> Draw reflections
// 2. Offset 0                         -> Draw refraction & scene
typedef struct 
{
    u32 ubo_min_alignment;
    u32 global_data;
    u32 ubo_gd_size;
    u32 light_data;
    u32 ubo_lt_size;
} Renderer;

void renderer_init(Renderer *renderer);
void renderer_free(Renderer *renderer);

void render_component_init(RenderComponent *comp, b8 indices, DrawMode mode);
void render_component_free(RenderComponent *comp);
void render_component_draw(RenderComponent *comp);
/* Assumes the shader is bound */
void renderer_prepare_shader_uniforms(ShaderProgram *shader);

#endif //_RENDERER_H
