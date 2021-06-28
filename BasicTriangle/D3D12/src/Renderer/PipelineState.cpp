
enum class GfxShaderStage : u8
{
    Vertex,
    Pixel,
    
    Count,
    Unknown = Count,
};

struct GfxShaderModules
{
    struct GfxShaderBlob *vertex = 0;
    struct GfxShaderBlob *pixel  = 0;
    // TODO(Dustin): Other shader modules
};

struct GfxShaderBlob
{
    ID3DBlob *handle;
};

enum class GfxFillMode
{
    Wireframe,
    Solid,
};

enum class GfxCullMode
{
    None,
    Front,
    Back,
};

struct GfxRasterDesc
{
    GfxFillMode fill_mode;
    GfxCullMode cull_mode;
    bool        front_count_clockwise;
    /* Omit depth bias? */
    /* Omit depth bias clamp? */
    /* Omit slope scaled depth bias? */
    bool       depth_clip_enabled;
    bool       multisample_enabled;
    bool       antialiased_enabled;
    u32        forced_sample_count;
    /* Omit conservative raster mode */
}; 

enum class GfxFormat : u8
{
    R32_Float,
    R32G32_Float,
    R32G32B32_Float,
    R32G32B32A32_Float,
    // TODO(Dustin): Others on a as-needed basis
    
    // Defaults for a user to specify these formats w/out
    // knowing their actual format
    Swapchain,
    Swapchain_DSV,
};

enum class GfxInputClass : u8
{
    PerVertex,
    PerInstance,
};

enum class GfxTopology
{
    Undefined,
    Point,
    Line,
    Triangle,
    Patch,
};

/* Specifies multisampling settings */
struct GfxSampleDesc
{
    u32 count;
    u32 quality;
};

struct GfxInputElementDesc
{
    const char   *semantic_name;
    u32           semantic_index;
    GfxFormat     format;
    u32           input_slot;
    u32           aligned_byte_offset;
    GfxInputClass input_class;
    u32           input_step_rate;
};

struct GfxPipelineStateDesc
{
    RootSignature      *root_signature; // TODO(Dustin): Root sig id?
    GfxShaderModules    shader_modules;
    /* Omit stream output desc */
    /* Omit blend desc */
    /* Omit sample mask */
    GfxRasterDesc       raster;
    /* Omit depth stencil state */
    // TODO(Dustin): Expose depth stencil
    GfxInputElementDesc *input_layouts;
    u32                  input_layouts_count;
    /* Omit index buffer strip cut value */
    GfxTopology         topology;
    /* When target count is 0, swapchain image is used by default */
    u32                 render_target_count;
    GfxFormat           rtv_formats[8];
    GfxFormat           dsv_format;
    GfxSampleDesc       sample_desc;
    /* Omit node mask */
    /* Omit pipeline cache state */
    /* Omit Flags */
};

struct PipelineStateObject
{
    ID3D12PipelineState *_handle;
    
    RenderError Init(GfxPipelineStateDesc *pso_desc);
    RenderError Free();
};

FORCE_INLINE GfxRasterDesc
GetDefaultRasterDesc()
{
    GfxRasterDesc desc{};
    desc.fill_mode = GfxFillMode::Solid;
    desc.cull_mode = GfxCullMode::None;
    desc.front_count_clockwise = false;
    desc.depth_clip_enabled = true;
    desc.multisample_enabled = false;
    desc.antialiased_enabled = false;
    desc.forced_sample_count = 0;
    return desc;
}

// TODO(Dustin): Export?
RenderError LoadShaderModules(GfxShaderModules *modules, i32 stage_count, ...);
RenderError CreatePipelineState(PipelineStateObject *pso, GfxPipelineStateDesc *pso_desc);

RenderError 
LoadShaderModules(GfxShaderModules *modules, i32 stages, ...)
{
    RenderError result = RenderError::Success;
    
    va_list list;
    va_start(list, stages);
    
    for (int i = 0; i < stages; ++i)
    {
        GfxShaderStage stage = va_arg(list, GfxShaderStage);
        wchar_t *file = va_arg(list, wchar_t*);
        
        ID3DBlob *shader_blob;
        JMP_FAILED(D3DReadFileToBlob(file, &shader_blob),
                   RenderError::ShaderBlobError);
        
        if (stage == GfxShaderStage::Vertex)
        {
            modules->vertex = (GfxShaderBlob*)shader_blob;
        }
        else if (stage == GfxShaderStage::Pixel)
        {
            modules->pixel = (GfxShaderBlob*)shader_blob;
        }
    }
    
    LBL_FAIL:;
    return result;
}

FORCE_INLINE D3D12_FILL_MODE
ToD3DFillMode(GfxFillMode mode)
{
    D3D12_FILL_MODE result = D3D12_FILL_MODE_SOLID;
    if (mode == GfxFillMode::Wireframe)
        result = D3D12_FILL_MODE_WIREFRAME;
    return result;
}

FORCE_INLINE D3D12_CULL_MODE
ToD3DCullMode(GfxCullMode mode)
{
    D3D12_CULL_MODE result = D3D12_CULL_MODE_NONE;
    if (mode == GfxCullMode::Front)
        result = D3D12_CULL_MODE_FRONT;
    else if (mode == GfxCullMode::Back)
        result = D3D12_CULL_MODE_BACK;
    return result;
}

FORCE_INLINE DXGI_FORMAT
ToDXGIFormat(GfxFormat format)
{
    DXGI_FORMAT result = DXGI_FORMAT_UNKNOWN;
    if (format == GfxFormat::R32_Float)
        result = DXGI_FORMAT_R32_FLOAT;
    else if (format == GfxFormat::R32G32_Float)
        result = DXGI_FORMAT_R32G32_FLOAT;
    else if (format == GfxFormat::R32G32B32_Float)
        result = DXGI_FORMAT_R32G32B32_FLOAT;
    else if (format == GfxFormat::R32G32B32A32_Float)
        result = DXGI_FORMAT_R32G32B32A32_FLOAT;
    else if (format == GfxFormat::Swapchain)
        result = DXGI_FORMAT_R8G8B8A8_UNORM;
    else if (format == GfxFormat::Swapchain_DSV)
        result = depth_buffer_format;
    return result;
}

FORCE_INLINE D3D12_INPUT_CLASSIFICATION 
ToD3DInputClass(GfxInputClass input)
{
    D3D12_INPUT_CLASSIFICATION result = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    if (input == GfxInputClass::PerInstance)
        result = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
    return result;
}

FORCE_INLINE D3D12_PRIMITIVE_TOPOLOGY_TYPE 
ToD3DTopologyType(GfxTopology top)
{
    D3D12_PRIMITIVE_TOPOLOGY_TYPE result = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    if (top == GfxTopology::Point)
        result = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    else if (top == GfxTopology::Line)
        result = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    else if (top == GfxTopology::Triangle)
        result = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    else if (top == GfxTopology::Patch)
        result = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    return result;
}

RenderError 
PipelineStateObject::Init(GfxPipelineStateDesc *pso_desc)
{
    RenderError result = RenderError::Success;
    
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = pso_desc->root_signature->_handle;
    
    D3D12_BLEND_DESC BlendDesc;
    BlendDesc.AlphaToCoverageEnable = FALSE;
    BlendDesc.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
    {
        FALSE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        BlendDesc.RenderTarget[i] = defaultRenderTargetBlendDesc;
    psoDesc.BlendState = BlendDesc;
    
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    
    D3D12_RASTERIZER_DESC RasterDesc{};
    RasterDesc.FillMode              = ToD3DFillMode(pso_desc->raster.fill_mode);
    RasterDesc.CullMode              = ToD3DCullMode(pso_desc->raster.cull_mode);
    RasterDesc.FrontCounterClockwise = pso_desc->raster.front_count_clockwise;
    RasterDesc.DepthClipEnable       = pso_desc->raster.depth_clip_enabled;
    RasterDesc.MultisampleEnable     = pso_desc->raster.multisample_enabled;
    RasterDesc.AntialiasedLineEnable = pso_desc->raster.antialiased_enabled;
    RasterDesc.ForcedSampleCount     = pso_desc->raster.forced_sample_count;
    RasterDesc.DepthBias             = D3D12_DEFAULT_DEPTH_BIAS;
    RasterDesc.DepthBiasClamp        = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    RasterDesc.SlopeScaledDepthBias  = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    RasterDesc.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    psoDesc.RasterizerState = RasterDesc;
    
    // @MALLOC
    D3D12_INPUT_ELEMENT_DESC *InputElementDescs = (D3D12_INPUT_ELEMENT_DESC*)malloc(sizeof(D3D12_INPUT_ELEMENT_DESC) * pso_desc->input_layouts_count);
    for (u32 i = 0; i < pso_desc->input_layouts_count; ++i)
    {
        GfxInputElementDesc *input = &pso_desc->input_layouts[i];
        InputElementDescs[i] = {
            input->semantic_name, 
            input->semantic_index,
            ToDXGIFormat(input->format),
            input->input_slot,
            input->aligned_byte_offset,
            ToD3DInputClass(input->input_class),
            input->input_step_rate,
        };
    }
    psoDesc.InputLayout = { InputElementDescs, pso_desc->input_layouts_count };
    
    GfxShaderModules *modules = &pso_desc->shader_modules;
    if (modules->vertex)
    {
        psoDesc.VS = { 
            reinterpret_cast<UINT8*>(((ID3DBlob*)modules->vertex)->GetBufferPointer()), 
            ((ID3DBlob*)modules->vertex)->GetBufferSize() 
        };
    }
    
    if (modules->pixel)
    {
        psoDesc.PS = { 
            reinterpret_cast<UINT8*>(((ID3DBlob*)modules->pixel)->GetBufferPointer()), 
            ((ID3DBlob*)modules->pixel)->GetBufferSize() 
        };
    }
    
    // TODO(Dustin): Other shader stages
    
    
    psoDesc.PrimitiveTopologyType = ToD3DTopologyType(pso_desc->topology);
    psoDesc.SampleDesc.Count = pso_desc->sample_desc.count;
    psoDesc.SampleDesc.Quality = pso_desc->sample_desc.quality;
    
    assert(pso_desc->render_target_count <= 8);
    psoDesc.NumRenderTargets = pso_desc->render_target_count;
    for (u32 i = 0; i < psoDesc.NumRenderTargets; ++i)
        psoDesc.RTVFormats[i] = ToDXGIFormat(pso_desc->rtv_formats[i]);
    
    JMP_FAILED(d3d_device->CreateGraphicsPipelineState(&psoDesc, IIDE(&_handle)),
               RenderError::PipelineStateError);
    
    LBL_FAIL:;
    free(InputElementDescs);
    return result;
}

RenderError
PipelineStateObject::Free()
{
    RenderError result = RenderError::Success;
    D3D_RELEASE(_handle);
    return result;
}