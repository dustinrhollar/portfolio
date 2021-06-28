#ifndef _RAYTRACER_RENDERER_H
#define _RAYTRACER_RENDERER_H

constexpr i32 RT_FRAME_COUNT = 2;
struct RtRenderer
{
    GfxPipeline       pipeline;
    GfxPipelineLayout layout;
    
    u32 tx_width, tx_height;
    volatile u32 rt_index;
    void        *rt_backing[RT_FRAME_COUNT];
    GfxTexture   rt_textures[RT_FRAME_COUNT];
};

file_internal void rt_renderer_init(RtRenderer *renderer);
file_internal void rt_renderer_free(RtRenderer *renderer);
file_internal void rt_renderer_copy(RtRenderer *renderer);
file_internal void rt_renderer_draw(RtRenderer *renderer);

#endif //_RAYTRACER_RENDERER_H
