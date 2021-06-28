#ifndef _WATER_GEN_H
#define _WATER_GEN_H

typedef struct 
{
    v2 pos;
    v4 indicators;
} WaterVertex;

typedef struct 
{
    ShaderProgram shader;
    Framebuffer   reflection_fbo;
    Framebuffer   refraction_fbo;
    // Uniforms
    v4            reflection_clip;
    v4            refraction_clip;
} WaterRenderer;

typedef struct
{
    WaterRenderer  *renderer;
    RenderComponent render_comp;
    r32             height;
    r32             wave_time;
} Water;

void water_renderer_init(
        WaterRenderer *renderer, 
        v2 reflection_fbo_dims,
        v2 refraction_fbo_dims,
        v4 reflection_clipping_plane, 
        v4 refraction_clipping_plane);
void water_renderer_free(WaterRenderer *renderer);

void water_gen_mesh(Water *water, WaterRenderer *renderer, u32 grid_length, r32 height);
void water_free(Water *water);
void water_draw(Water *water);

#endif
