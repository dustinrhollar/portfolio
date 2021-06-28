#ifndef _DX11_RENDERER_H
#define _DX11_RENDERER_H

extern struct ID3D11Device            *g_device;
extern struct ID3D11DeviceContext     *g_device_ctx;
extern struct IDXGISwapChain          *g_swapchain;
extern struct ID3D11RenderTargetView  *g_gfx_primary_rt;
extern struct ID3D11DepthStencilView  *g_gfx_depth_stencil_view;
extern struct ID3D11Texture2D         *g_gfx_depth_stencil_buffer;
extern struct ID3D11DepthStencilState *g_gfx_depth_stencil_state;

typedef struct 
{
    const char            *name;
    u32                    semantic_index; // i.e. if name is COLOR1, then semantic index is 1
    GfxInputFormat         input_format;
    /* The IA stage has n input slots, which are designed to accommodate up to n vertex buffers 
that provide input data. Each vertex buffer must be assigned to a different slot; this 
information is stored in the input-layout declaration when the input-layout object is created.  */
    u32                    input_slot;
    u32                    offset;
    b8                     per_vertex; // true if stride is per vertex
    u32                    instance_rate;
} GfxPipelineInputInfo;

typedef struct 
{
    GfxShaderType          type;
    const char            *entry_ptr;
} GfxPipelineStage;

typedef struct 
{
    struct ID3D10Blob* handle;
} GfxShaderBlob;

typedef struct
{
    GfxShaderBlob    *blob;
    GfxPipelineStage *stages;
    u32               stages_count;
} GfxPipelineCreateInfo;

typedef struct
{
    GfxShaderBlob               *blob;
    GfxPipelineInputInfo        *pipeline_layouts;
    u32                          pipeline_layouts_count;
} GfxPipelineLayoutCreateInfo;

typedef struct
{
    struct ID3D11InputLayout    *handle;
} GfxPipelineLayout;

typedef struct
{
    struct ID3D11VertexShader     *vertex;
    struct ID3D11PixelShader      *pixel;
    struct ID3D11GeometryShader   *geometry;
    struct ID3D11HullShader       *hull;
    struct ID3D11DomainShader     *domain;
    struct ID3D11ComputeShader    *compute;
} GfxPipeline;

typedef enum
{
} DrawMode;

typedef struct
{
    u32                     size;
    GfxBufferUsage          usage;
    GfxCpuAccessFlags       cpu_access_flags;
    GfxBufferBindFlags      bind_flags;
    GfxBufferMiscFlags      misc_flags;
    u32                     structure_byte_stride;
    
    // Optional data
    const void             *data;
    u32                     sys_mem_pitch;
    u32                     sys_mem_slice_pitch;
} GfxBufferCreateInfo;

typedef struct
{
    struct ID3D11Buffer *handle;
} GfxBuffer;

typedef struct 
{
    
} GfxRenderComponent;

// Stride is always aligned to 128bits
// Vector types should be internally aligned
// - Aligned to vec4 (128bits)
// Some good commentary on Structured Buffers vs
// Constant Buffers
// https://developer.nvidia.com/content/how-about-constant-buffers
//
// Should generally be used data > 64kb & for incoherent access
// patterns. There is a lot of latency for accessing structured buffers
typedef struct
{
} GfxStructuredBuffer;


// Max Size: 64kb
// For large CB with partial updates:
// https://developer.nvidia.com/content/constant-buffers-without-constant-pain-0
typedef struct 
{
} GfxConstantBuffer;

void renderer_init(u32 width, u32 height, u32 refresh_rate, u32 multisample_count, HostWnd *wnd);
void renderer_free();
void renderer_resize(u32 width, u32 height);

b8   gfx_shader_blob_init(GfxShaderBlob *blob, const char16_t *file);
void gfx_shader_blob_free(GfxShaderBlob *blob);

// shader_program_init(shader, GfxShader_Vertex, GfxShaderBlob, GfxShader_Pixel, GfxShaderBlob)
void gfx_pipeline_init(GfxPipeline *pipeline, i32 stages, ...);
void gfx_pipeline_free(GfxPipeline *pipeline);

void gfx_pipeline_layout_init(GfxPipelineLayout *layout, GfxShaderBlob *vtx_blob, GfxPipelineInputInfo *infos, u32 infos_count);
void gfx_pipeline_layout_free(GfxPipelineLayout *layout);

void gfx_buffer_init(GfxBuffer *buffer, GfxBufferCreateInfo *info);
void gfx_buffer_free(GfxBuffer *buffer);

#endif //_D_X11_RENDERER_H
