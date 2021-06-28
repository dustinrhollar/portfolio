
void gfx_texture_init(GfxTexture *texture, GfxTextureCreateInfo *info)
{
    D3D11_TEXTURE2D_DESC desc;
    memset(&desc, 0, sizeof(desc));
    
    // TODO(Dustin): Handle mipping
    desc.Width            = info->width;
    desc.Height           = info->height;
    desc.MipLevels        = 1;
    desc.ArraySize        = 1;
    desc.SampleDesc.Count = 1;
    
    switch (info->format)
    {
        case PipelineFormat_R32_FLOAT:          desc.Format = DXGI_FORMAT_R32_FLOAT;          break;
        case PipelineFormat_R32G32_FLOAT:       desc.Format = DXGI_FORMAT_R32G32_FLOAT;       break;
        case PipelineFormat_R32G32B32_FLOAT:    desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;    break;
        case PipelineFormat_R32G32B32A32_FLOAT: desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
        case PipelineFormat_R32G32B32_UINT:     desc.Format = DXGI_FORMAT_R32G32B32_UINT;     break;
        case PipelineFormat_R32G32B32A32_UINT:  desc.Format = DXGI_FORMAT_R32G32B32A32_UINT;  break;
        default: LogError("Format not currently supported!\n");
    }
    
    switch (info->usage)
    {
        case BufferUsage_Default:
        {
            desc.Usage = D3D11_USAGE_DEFAULT; // write access access by CPU and GPU
        } break;
        
        case BufferUsage_Immutable:
        {
            desc.Usage = D3D11_USAGE_IMMUTABLE;
        } break;
        
        case BufferUsage_Dynamic:
        {
            desc.Usage = D3D11_USAGE_DYNAMIC;
        } break;
        
        case BufferUsage_Staging:
        {
            desc.Usage = D3D11_USAGE_STAGING;
        } break;
        
        default: break;
    }
    
    desc.BindFlags = 0;
    if (info->bind_flags & BufferBind_VertexBuffer)
        desc.BindFlags |= D3D11_BIND_VERTEX_BUFFER;
    if (info->bind_flags & BufferBind_IndexBuffer)
        desc.BindFlags |= D3D11_BIND_INDEX_BUFFER;
    if (info->bind_flags & BufferBind_ConstantBuffer)
        desc.BindFlags |= D3D11_BIND_CONSTANT_BUFFER;
    if (info->bind_flags & BufferBind_ShaderResource)
        desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    if (info->bind_flags & BufferBind_StreamOutput)
        desc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
    if (info->bind_flags & BufferBind_RenderTarget)
        desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    if (info->bind_flags & BufferBind_DepthStencil)
        desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
    if (info->bind_flags & BufferBind_UnorderedAccess)
        desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    
    desc.CPUAccessFlags = 0;
    if (info->cpu_access_flags & BufferCpuAccess_Write)
        desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
    if (info->cpu_access_flags & BufferCpuAccess_Read)
        desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
    
    desc.MiscFlags = 0;
    if (info->misc_flags & BufferMisc_GenMips)
        desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    
    HRESULT hr;
    if (info->data)
    {
        D3D11_SUBRESOURCE_DATA InitData = {0};
        InitData.pSysMem          = info->data;
        InitData.SysMemPitch      = info->sys_mem_pitch;
        InitData.SysMemSlicePitch = info->sys_mem_slice_pitch;
        hr = g_device->CreateTexture2D(&desc, &InitData, &texture->handle);
    }
    else
    {
        hr = g_device->CreateTexture2D(&desc, NULL, &texture->handle);
    }
    
    
    if (FAILED(hr))
    {
        LogError("Failed to create 2D image texture %d!\n", hr);
        return;
    }
    
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    memset( &SRVDesc, 0, sizeof( SRVDesc ) );
    
    SRVDesc.Format = desc.Format;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MipLevels = 1;
    
    hr = g_device->CreateShaderResourceView(texture->handle, &SRVDesc, &texture->view);
    if (FAILED(hr))
    {
        LogError("Failed to create 2D image texture image view %d!\n", hr);
        texture->handle->Release();
        return;
    }
    
    {
        D3D11_SAMPLER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.MipLODBias = 0.f;
        desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        desc.MinLOD = 0.f;
        desc.MaxLOD = 0.f;
        hr = g_device->CreateSamplerState(&desc, &texture->sampler);
        if (FAILED(hr))
        {
            LogError("Failed to sampler for the image %d!\n", hr);
            
            texture->handle->Release();
            texture->view->Release();
            return;
        }
    }
}

void gfx_texture_free(GfxTexture *texture)
{
    if (texture->handle) texture->handle->Release();
    if (texture->view) texture->view->Release();
    if (texture->sampler) texture->sampler->Release();
    
    texture->handle = 0;
    texture->view = 0;
    texture->sampler = 0;
}

GfxTextureMapped gfx_texture_map(GfxTexture *texture, GfxMapFlags flags)
{
    D3D11_MAP map;
    if (flags == GfxMapFlags::WriteDiscard) map = D3D11_MAP_WRITE_DISCARD;
    // TODO(Dustin): Do the other flags at some point...
    
    D3D11_MAPPED_SUBRESOURCE rsrc;
    ThrowIfFailed(g_device_ctx->Map(texture->handle, 0, map,0,&rsrc),
                  "Failed to map perlin display texture!");
    
    
    GfxTextureMapped result = {};
    result.data = rsrc.pData;
    result.row_pitch = rsrc.RowPitch;
    result.depth_pitch = rsrc.DepthPitch;
    return result;
}

void gfx_texture_unmap(GfxTexture *texture)
{
    g_device_ctx->Unmap(texture->handle, 0);
}