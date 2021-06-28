#ifndef _DX11_COMMON_H
#define _DX11_COMMON_H

typedef enum
{
    GfxShader_Vertex,
    GfxShader_Pixel,
    GfxShader_Geometry,
    GfxShader_Hull,
    GfxShader_Domain,
    GfxShader_Compute,
    
    GfxShader_Count,
    GfxShader_Unknown = GfxShader_Count,
} GfxShaderType;

typedef enum
{
    PipelineFormat_R32_FLOAT,
    PipelineFormat_R32G32_FLOAT,
    PipelineFormat_R32G32B32_FLOAT,
    PipelineFormat_R32G32B32A32_FLOAT,
    PipelineFormat_R32G32B32_UINT,
    PipelineFormat_R32G32B32A32_UINT,
    
    PipelineFormat_Count,
    PipelineFormat_Unknown = PipelineFormat_Count,
} GfxInputFormat;

typedef enum 
{
    BufferUsage_Default,
    BufferUsage_Immutable,
    BufferUsage_Dynamic,
    BufferUsage_Staging,
} GfxBufferUsage;

typedef enum
{ // TODO(Dustin): Will these ever be OR'd together?
    BufferCpuAccess_None  = BIT(0),
    BufferCpuAccess_Write = BIT(1),
    BufferCpuAccess_Read  = BIT(2),
} GfxCpuAccessFlags;

// NOTE(Dustin): Add to on a as-needed basis
typedef enum
{
    BufferMisc_None    = BIT(0),
    BufferMisc_GenMips = BIT(1),
} GfxBufferMiscFlags;

typedef enum
{
    BufferBind_None            = BIT(0),
    BufferBind_VertexBuffer    = BIT(1),
    BufferBind_IndexBuffer     = BIT(2),
    BufferBind_ConstantBuffer  = BIT(3),
    BufferBind_ShaderResource  = BIT(4),
    BufferBind_StreamOutput    = BIT(5),
    BufferBind_RenderTarget    = BIT(6),
    BufferBind_DepthStencil    = BIT(7),
    BufferBind_UnorderedAccess = BIT(8),
} GfxBufferBindFlags;

#endif //_DX11_COMMON_H
