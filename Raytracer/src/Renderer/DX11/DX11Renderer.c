
IDXGISwapChain          *g_swapchain = 0;
ID3D11Device            *g_device = 0;
ID3D11DeviceContext     *g_device_ctx = 0;
ID3D11RenderTargetView  *g_gfx_primary_rt = 0;
ID3D11DepthStencilView  *g_gfx_depth_stencil_view = 0;
ID3D11Texture2D         *g_gfx_depth_stencil_buffer = 0;
ID3D11DepthStencilState *g_gfx_depth_stencil_state = 0;

file_internal void renderer_setup_offscreen_buffers(u32 width, u32 height);

void renderer_init(u32 width, u32 height, UINT refresh_rate, UINT multisample_count, HostWnd *wnd)
{
    // NOTE(Dustin): Notes on resizing...
    // IDXGISwapChain::ResizeBuffers will resize the buffer, but need to release pointers before hand
    // IDXGIFactory::MakeWindowAssociation is useful for transition between windowed and fullscreen
    //                                     with the Alt-Enter key combination.
    // More on swpachain can be found here:
    // https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/d3d10-graphics-programming-guide-dxgi
    
    // TODO(Dustin): Determine swapchain backbuffer information. Not sure what it should be set to
    // in D3D11.
    
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount                        = 1;
    sd.BufferDesc.Width                   = width;
    sd.BufferDesc.Height                  = height;
    sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = refresh_rate;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                       = wnd->handle;
    sd.SampleDesc.Count                   = multisample_count;
    sd.SampleDesc.Quality                 = 0;
    sd.Windowed                           = TRUE;
    sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // allow full-screen switching
    
    D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    
    UINT CreationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
    // If the project is in a debug build, enable the debug layer.
    CreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    
    HRESULT hr = S_OK;
    D3D_FEATURE_LEVEL FeatureLevel;
    
    hr = D3D11CreateDeviceAndSwapChain(NULL,
                                       D3D_DRIVER_TYPE_REFERENCE,
                                       NULL,
                                       CreationFlags,
                                       FeatureLevels,
                                       1,
                                       D3D11_SDK_VERSION,
                                       &sd,
                                       &g_swapchain,
                                       &g_device,
                                       &FeatureLevel,
                                       &g_device_ctx);
    
    if (FAILED(hr))
    {
        LogFatal("Failed to create Device and swapchain because of %d!\n", hr);
    }
    
    // Create the primary render target
    
    // get the address of the back buffer
    ID3D11Texture2D *tBackBuffer;
    hr = g_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&tBackBuffer);
    if (FAILED(hr))
    {
        LogFatal("Failed to get back buffer texture %d!\n", hr);
    }
    else
    {
        // use the back buffer address to create the render target
        hr = g_device->CreateRenderTargetView(tBackBuffer, NULL, &g_gfx_primary_rt);
        tBackBuffer->Release();
        if (FAILED(hr))
        {
            LogFatal("Failed to create render target view %d!\n", hr);
        }
    }
    
    renderer_setup_offscreen_buffers(width, height);
}

file_internal void renderer_setup_offscreen_buffers(u32 width, u32 height)
{
    // Create the Depth Stencil Information
    
    D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
    ZeroMemory( &depthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC) );
    
    depthStencilBufferDesc.ArraySize = 1;
    depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilBufferDesc.CPUAccessFlags = 0; // No CPU access required.
    depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilBufferDesc.Width = width;
    depthStencilBufferDesc.Height = height;
    depthStencilBufferDesc.MipLevels = 1;
    depthStencilBufferDesc.SampleDesc.Count = 1;
    depthStencilBufferDesc.SampleDesc.Quality = 0;
    depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    
    HRESULT hr = g_device->CreateTexture2D(&depthStencilBufferDesc, nullptr, &g_gfx_depth_stencil_buffer);
    if ( FAILED(hr) )
    {
        LogFatal("Failed to create the DepthStencil buffer!");
    }
    
    hr = g_device->CreateDepthStencilView(g_gfx_depth_stencil_buffer, nullptr, &g_gfx_depth_stencil_view);
    if ( FAILED(hr) )
    {
        LogFatal("Failed to create the DepthStencil buffer view!");
    }
    
    // Setup depth/stencil state.
    D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
    ZeroMemory( &depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC) );
    
    depthStencilStateDesc.DepthEnable = TRUE;
    depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilStateDesc.StencilEnable = FALSE;
    
    hr = g_device->CreateDepthStencilState( &depthStencilStateDesc, &g_gfx_depth_stencil_state );
    if ( FAILED(hr) )
    {
        LogFatal("Failed to create the DepthStencil state!");
    }
}

void renderer_free()
{
    g_gfx_depth_stencil_view->Release();
    g_gfx_depth_stencil_buffer->Release();
    g_gfx_depth_stencil_state->Release();
    g_gfx_primary_rt->Release();
    g_swapchain->Release();
    g_device_ctx->Release();
    g_device->Release();
}

void renderer_resize(u32 width, u32 height)
{
    g_device_ctx->OMSetRenderTargets(0, 0, 0);
    g_gfx_primary_rt->Release();
    g_gfx_depth_stencil_view->Release();
    g_gfx_depth_stencil_buffer->Release();
    g_gfx_depth_stencil_state->Release();
    
    HRESULT hr = g_swapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr))
    {
        LogFatal("Failed to preserve swapchain format during resize!\n");
    }
    
    ID3D11Texture2D* pBuffer;
    hr = g_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &pBuffer );
    if (FAILED(hr))
    {
        LogFatal("Failed to retrieve the texture for the RenderTarget View!\n");
        return;
    }
    
    hr = g_device->CreateRenderTargetView(pBuffer, NULL, &g_gfx_primary_rt);
    if (FAILED(hr))
    {
        LogFatal("Failed to create the RenderTarget View!\n");
        return;
    }
    pBuffer->Release();
    
    renderer_setup_offscreen_buffers(width, height);
}

b8 gfx_shader_blob_init(GfxShaderBlob *blob, const char16_t *file)
{
    ID3DBlob* shaderBlob = 0;
    HRESULT hr = D3DReadFileToBlob((LPCWSTR)file, &shaderBlob);
    
    if ( FAILED(hr) )
    {
        LogError("Failed to read file to blob!");
        
        if (shaderBlob)
            shaderBlob->Release();
        
        return false;
    }    
    
    blob->handle = shaderBlob;
    return true;
}

void gfx_shader_blob_free(GfxShaderBlob *blob)
{
    blob->handle->Release();
}

// shader_program_init(shader, GfxShader_Vertex, GfxShaderBlob, GfxShader_Pixel, GfxShaderBlob)
void gfx_pipeline_init(GfxPipeline *pipeline, i32 stages, ...)
{
    *pipeline = {}; // zero init
    
    va_list list;
    va_start(list, stages);
    
    for (int i = 0; i < stages; ++i)
    {
        GfxShaderType stage = va_arg(list, GfxShaderType);
        GfxShaderBlob *blob = va_arg(list, GfxShaderBlob*);
        
        switch (stage)
        {
            case GfxShader_Vertex:
            {
                ThrowIfFailed(g_device->CreateVertexShader(blob->handle->GetBufferPointer(),
                                                           blob->handle->GetBufferSize(),
                                                           0,
                                                           &pipeline->vertex),
                              "Failed to create vertex shader!");
            } break;
            
            case GfxShader_Pixel:
            {
                ThrowIfFailed(g_device->CreatePixelShader(blob->handle->GetBufferPointer(),
                                                          blob->handle->GetBufferSize(),
                                                          0,
                                                          &pipeline->pixel),
                              "Failed to create vertex shader!");
            } break;
            
            case GfxShader_Geometry:
            {
                ThrowIfFailed(g_device->CreateGeometryShader(blob->handle->GetBufferPointer(),
                                                             blob->handle->GetBufferSize(),
                                                             0,
                                                             &pipeline->geometry),
                              "Failed to create vertex shader!");
            } break;
            
            case GfxShader_Hull:
            {
                ThrowIfFailed(g_device->CreateHullShader(blob->handle->GetBufferPointer(),
                                                         blob->handle->GetBufferSize(),
                                                         0,
                                                         &pipeline->hull),
                              "Failed to create vertex shader!");
            } break;
            
            case GfxShader_Domain:
            {
                ThrowIfFailed(g_device->CreateDomainShader(blob->handle->GetBufferPointer(),
                                                           blob->handle->GetBufferSize(),
                                                           0,
                                                           &pipeline->domain),
                              "Failed to create vertex shader!");
            } break;
            
            case GfxShader_Compute:
            {
                ThrowIfFailed(g_device->CreateComputeShader(blob->handle->GetBufferPointer(),
                                                            blob->handle->GetBufferSize(),
                                                            0,
                                                            &pipeline->compute),
                              "Failed to create vertex shader!");
            } break;
        }
    }
    
    va_end(list);
}

void gfx_pipeline_free(GfxPipeline *pipeline)
{
    if (pipeline->vertex)   pipeline->vertex->Release();
    if (pipeline->pixel)    pipeline->pixel->Release();
    if (pipeline->geometry) pipeline->geometry->Release();
    if (pipeline->hull)     pipeline->hull->Release();
    if (pipeline->domain)   pipeline->domain->Release();
    if (pipeline->compute)  pipeline->compute->Release();
}

void gfx_pipeline_layout_init(GfxPipelineLayout *layout, GfxShaderBlob *vtx_blob, GfxPipelineInputInfo *infos, u32 infos_count)
{
    // 16 inputs are based on valid input slots for D3D11_INPUT_ELEMENT_DESC
    AssertCustom(infos_count <= 16, "Arbirtrary limitation on shader inputs...");
    
    D3D11_INPUT_ELEMENT_DESC ied[16] = {};
    for (u32 i = 0; i < infos_count; ++i)
    {
        ied[i] = {};
        ied[i].SemanticName = infos[i].name;
        ied[i].SemanticIndex = infos[i].semantic_index;
        
        switch (infos[i].input_format)
        {
            case PipelineFormat_R32G32_FLOAT:       ied[i].Format = DXGI_FORMAT_R32G32_FLOAT;       break;
            case PipelineFormat_R32G32B32_FLOAT:    ied[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;    break;
            case PipelineFormat_R32G32B32A32_FLOAT: ied[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
            default: LogError("Format not currently supported!\n");
        }
        
        ied[i].InputSlot = infos[i].input_slot;
        ied[i].AlignedByteOffset = infos[i].offset;
        
        if (infos[i].per_vertex)
            ied[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        else
            ied[i].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
        
        ied[i].InstanceDataStepRate = infos[i].instance_rate;
    }
    
    HRESULT hr = g_device->CreateInputLayout(ied, infos_count,
                                             vtx_blob->handle->GetBufferPointer(),
                                             vtx_blob->handle->GetBufferSize(),
                                             &layout->handle);
    if (FAILED(hr))
    {
        LogError("Failed to create pipeline layout %d!\n", hr);
    }
}

void gfx_pipeline_layout_free(GfxPipelineLayout *layout)
{
    layout->handle->Release();
}

void gfx_buffer_init(GfxBuffer *buffer, GfxBufferCreateInfo *info)
{
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.ByteWidth = info->size;
    
    switch (info->usage)
    {
        case BufferUsage_Default:
        {
            bd.Usage = D3D11_USAGE_DEFAULT; // write access access by CPU and GPU
        } break;
        
        case BufferUsage_Immutable:
        {
            bd.Usage = D3D11_USAGE_IMMUTABLE;
        } break;
        
        case BufferUsage_Dynamic:
        {
            bd.Usage = D3D11_USAGE_DYNAMIC;
        } break;
        
        case BufferUsage_Staging:
        {
            bd.Usage = D3D11_USAGE_STAGING;
        } break;
        
        default: break;
    }
    
    bd.BindFlags = 0;
    if (info->bind_flags & BufferBind_VertexBuffer)
        bd.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
    if (info->bind_flags & BufferBind_IndexBuffer)
        bd.BindFlags |= D3D11_BIND_INDEX_BUFFER;
    if (info->bind_flags & BufferBind_ConstantBuffer)
        bd.BindFlags |= D3D11_BIND_CONSTANT_BUFFER;
    if (info->bind_flags & BufferBind_ShaderResource)
        bd.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    if (info->bind_flags & BufferBind_StreamOutput)
        bd.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
    if (info->bind_flags & BufferBind_RenderTarget)
        bd.BindFlags |= D3D11_BIND_RENDER_TARGET;
    if (info->bind_flags & BufferBind_DepthStencil)
        bd.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
    if (info->bind_flags & BufferBind_UnorderedAccess)
        bd.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    
    bd.CPUAccessFlags = 0;
    if (info->cpu_access_flags & BufferCpuAccess_Write)
        bd.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
    if (info->cpu_access_flags & BufferCpuAccess_Read)
        bd.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
    
    bd.MiscFlags = 0;
    if (info->misc_flags & BufferMisc_GenMips)
        bd.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    
    // Fill in the subresource data.
    HRESULT hr;
    if (info->data)
    {
        D3D11_SUBRESOURCE_DATA InitData = {0};
        InitData.pSysMem          = info->data;
        InitData.SysMemPitch      = info->sys_mem_pitch;
        InitData.SysMemSlicePitch = info->sys_mem_slice_pitch;
        
        hr = g_device->CreateBuffer(&bd, &InitData, &buffer->handle);
    }
    else
    {
        hr = g_device->CreateBuffer(&bd, NULL, &buffer->handle);
    }
    
    if (FAILED(hr))
    {
        LogError("Failed to create vertex buffer %d!\n", hr);
    }
}

void gfx_buffer_free(GfxBuffer *buffer)
{
    buffer->handle->Release();
}

void gfx_render_component_init(GfxRenderComponent *render_comp, b8 indices, DrawMode mode)
{
}