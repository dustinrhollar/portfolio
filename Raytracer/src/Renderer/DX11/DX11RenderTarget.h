#ifndef _DX11_RENDER_TARGET_H
#define _DX11_RENDER_TARGET_H

struct GfxRenderTarget
{
    struct ID3D11Texture2D          *handle = 0;
    struct ID3D11RenderTargetView   *view = 0;
    struct ID3D11ShaderResourceView *srv = 0;
    struct ID3D11DepthStencilView   *ds_view = 0;
    struct ID3D11Texture2D          *ds_buffer = 0;
    struct ID3D11DepthStencilState  *ds_state = 0;
    u32                              width = 0;
    u32                              height = 0;
    b8                               has_depth = false;
};

void gfx_rt_init(GfxRenderTarget *rt, u32 width, u32 height, b8 has_depth = true);
void gfx_rt_free(GfxRenderTarget *rt);
void gfx_rt_resize(GfxRenderTarget *rt, u32 width, u32 height);
void gfx_rt_bind(GfxRenderTarget *rt, r32 clear_color[4], r32 clear_depth = 1.0f, i32 clear_stencil = 0);
void gfx_rt_unbind(GfxRenderTarget *rt);

#endif //_DX11_RENDER_TARGET_H
