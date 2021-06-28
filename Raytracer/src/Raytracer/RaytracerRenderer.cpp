
file_internal void 
rt_renderer_init(RtRenderer *renderer, u32 texture_width, u32 texture_height)
{
    renderer->tx_width = texture_width;
    renderer->tx_height = texture_height;
    
    char16_t *vfile = u"shaders/quad_vert.cso";
    char16_t *pfile = u"shaders/quad_frag.cso";
    
    GfxShaderBlob vblob, pblob;
    if (!gfx_shader_blob_init(&vblob, vfile)) 
        LogFatal("Failed to read file %s!", vfile);
    if (!gfx_shader_blob_init(&pblob, pfile)) 
        LogFatal("Failed to read file %s!", vfile);
    
    gfx_pipeline_init(&renderer->pipeline, 2, GfxShader_Vertex, &vblob, GfxShader_Pixel, &pblob);
    gfx_pipeline_layout_init(&renderer->layout, &vblob, NULL, 0);
    
    // RT Texture
    for (int i = 0; i < RT_FRAME_COUNT; ++i)
    {
        // Go ahead and create the backing memory...
        u64 size = sizeof(v3) * texture_width * texture_height;
        renderer->rt_backing[i] = malloc(size);
        memset(renderer->rt_backing[i], 1, size);
        
        GfxTextureCreateInfo info{};
        info.width = texture_width;
        info.height = texture_height;
        info.usage = BufferUsage_Dynamic;
        info.cpu_access_flags = BufferCpuAccess_Write;
        info.bind_flags = BufferBind_ShaderResource;
        info.misc_flags = BufferMisc_None;
        info.structure_byte_stride = 0;
        info.format = PipelineFormat_R32G32B32_FLOAT;
        
        // Optional data
        info.data = renderer->rt_backing[i];
        info.sys_mem_pitch = sizeof(v3) * texture_width;
        info.sys_mem_slice_pitch = 0;
        
        gfx_texture_init(&renderer->rt_textures[i], &info);
    }
    
    InterlockedExchange(&renderer->rt_index, 0);
}

file_internal void 
rt_renderer_free(RtRenderer *renderer)
{
    for (int i = 0; i < RT_FRAME_COUNT; ++i)
    {
        free(renderer->rt_backing[i]);
        gfx_texture_free(&renderer->rt_textures[i]);
    }
    
    gfx_pipeline_free(&renderer->pipeline);
}

file_internal void 
rt_renderer_copy(RtRenderer *renderer)
{
    // Copies into the *next frame* and updates the frame index
    
    u32 index = (renderer->rt_index + 1) % RT_FRAME_COUNT;
    void *read_texture = renderer->rt_backing[0];
    GfxTexture *write_texture = &renderer->rt_textures[index];
    
    D3D11_MAPPED_SUBRESOURCE rsrc;
    HRESULT hr = g_device_ctx->Map(write_texture->handle, 0, D3D11_MAP_WRITE_DISCARD, 0, &rsrc);
    if (FAILED(hr))
    {
        LogError("Failed to map the texture resource at setup!\n");
    }
    else
    {
        char* backbuffer = (char*)rsrc.pData;
        memcpy(backbuffer, read_texture, rsrc.DepthPitch);
        g_device_ctx->Unmap(write_texture->handle, 0);
        _InterlockedExchange(&renderer->rt_index, index);
    }
}

file_internal void 
rt_renderer_draw(RtRenderer *renderer)
{
    g_device_ctx->IASetInputLayout(renderer->layout.handle);
    g_device_ctx->VSSetShader(renderer->pipeline.vertex, 0, 0);
    g_device_ctx->PSSetShader(renderer->pipeline.pixel, 0, 0);
    
    GfxTexture *tex = &renderer->rt_textures[renderer->rt_index];
    g_device_ctx->PSSetShaderResources(0, 1, &tex->view);
    g_device_ctx->PSSetSamplers(0, 1, &tex->sampler);
    
    // select which primtive type we are using
    g_device_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    g_device_ctx->Draw(4, 0);
}
