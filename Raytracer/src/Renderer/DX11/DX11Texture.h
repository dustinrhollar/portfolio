#ifndef _DX11_TEXTURE_H
#define _DX11_TEXTURE_H

struct GfxTextureCreateInfo
{
    u32                width;
    u32                height;
    GfxBufferUsage     usage;
    GfxCpuAccessFlags  cpu_access_flags;
    GfxBufferBindFlags bind_flags;
    GfxBufferMiscFlags misc_flags;
    u32                structure_byte_stride;
    GfxInputFormat     format;
    
    // Optional data
    const void        *data;
    u32                sys_mem_pitch;
    u32                sys_mem_slice_pitch;
};

enum class GfxMapFlags
{
    WriteDiscard
};

FORCE_INLINE bool operator==(GfxMapFlags left, GfxMapFlags right)
{
    return (i32)left == (i32)right;
}

struct GfxTextureMapped
{
    void *data;
    u32   row_pitch;
    u32   depth_pitch;
};

struct GfxTexture
{
    struct ID3D11Texture2D          *handle;
    struct ID3D11ShaderResourceView *view;
    struct ID3D11SamplerState       *sampler;
};

void gfx_texture_init(GfxTexture *texture, GfxTextureCreateInfo *info);
void gfx_texture_free(GfxTexture *texture);
GfxTextureMapped gfx_texture_map(GfxTexture *texture, GfxMapFlags flags);
void gfx_texture_unmap(GfxTexture *texture);

#endif //_DX11_TEXTURE_H
