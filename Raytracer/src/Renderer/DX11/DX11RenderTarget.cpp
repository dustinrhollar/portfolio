
void gfx_rt_init(GfxRenderTarget *rt, u32 width, u32 height, b8 has_depth)
{
    *rt = {};
    rt->has_depth = has_depth;
    gfx_rt_resize(rt, width, height);
}

void gfx_rt_free(GfxRenderTarget *rt)
{
    if (rt->handle)    rt->handle->Release();
    if (rt->view)      rt->view->Release();
    if (rt->ds_view)   rt->ds_view->Release();
    if (rt->ds_buffer) rt->ds_buffer->Release();
    if (rt->ds_state)  rt->ds_state->Release();
    rt->width = rt->height = 0;
    rt->has_depth = false;
}

void gfx_rt_resize(GfxRenderTarget *rt, u32 width, u32 height)
{
    if (width == rt->width && height == rt->height) return;
    
    rt->width  = fast_max(1, width);
    rt->height = fast_max(1, height);
    
    if (rt->handle)    rt->handle->Release();
    if (rt->view)      rt->view->Release();
    
    D3D11_TEXTURE2D_DESC buffer_desc;
    buffer_desc.ArraySize = 1;
    buffer_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    buffer_desc.CPUAccessFlags = 0;
    buffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Same format as Swapchain?
    buffer_desc.Width = rt->width;
    buffer_desc.Height = rt->height;
    buffer_desc.MipLevels = 1;
    buffer_desc.MiscFlags = 0;
    buffer_desc.SampleDesc = {1, 0};
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    ThrowIfFailed(g_device->CreateTexture2D(&buffer_desc, 0, &rt->handle),
                  "Failed to create the render target texture!");
    
    D3D11_RENDER_TARGET_VIEW_DESC rt_view_desc;
    rt_view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rt_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rt_view_desc.Texture2D.MipSlice = 0;
    ThrowIfFailed(g_device->CreateRenderTargetView(rt->handle, &rt_view_desc, &rt->view),
                  "Failed to create the render target view!");
    
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
    srv_desc.Format = buffer_desc.Format;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.MipLevels = 1;
    ThrowIfFailed(g_device->CreateShaderResourceView(rt->handle, &srv_desc, &rt->srv),
                  "Failed to create the render target shader resource view!");
    
    if (rt->has_depth)
    {
        if (rt->ds_view)   rt->ds_view->Release();
        if (rt->ds_buffer) rt->ds_buffer->Release();
        if (rt->ds_state)  rt->ds_state->Release();
        
        D3D11_TEXTURE2D_DESC ds_buffer_desc;
        ZeroMemory(&ds_buffer_desc, sizeof(D3D11_TEXTURE2D_DESC));
        
        ds_buffer_desc.ArraySize = 1;
        ds_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        ds_buffer_desc.CPUAccessFlags = 0; // No CPU access required.
        ds_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        ds_buffer_desc.Width = rt->width;
        ds_buffer_desc.Height = rt->height;
        ds_buffer_desc.MipLevels = 1;
        ds_buffer_desc.SampleDesc.Count = 1;
        ds_buffer_desc.SampleDesc.Quality = 0;
        ds_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        
        HRESULT hr = g_device->CreateTexture2D(&ds_buffer_desc, 0, &rt->ds_buffer);
        if (FAILED(hr))
        {
            LogFatal("Failed to create the Render Target DepthStencil buffer!");
        }
        
        hr = g_device->CreateDepthStencilView(rt->ds_buffer, 0, &rt->ds_view);
        if (FAILED(hr))
        {
            LogFatal("Failed to create the Render Target DepthStencil buffer view!");
        }
        
        // Setup depth/stencil state.
        D3D11_DEPTH_STENCIL_DESC ds_state_desc;
        ZeroMemory( &ds_state_desc, sizeof(D3D11_DEPTH_STENCIL_DESC) );
        
        ds_state_desc.DepthEnable = TRUE;
        ds_state_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        ds_state_desc.DepthFunc = D3D11_COMPARISON_LESS;
        ds_state_desc.StencilEnable = FALSE;
        
        hr = g_device->CreateDepthStencilState(&ds_state_desc, &rt->ds_state);
        if ( FAILED(hr) )
        {
            LogFatal("Failed to create the Render Target DepthStencil state!");
        }
    }
}

void gfx_rt_bind(GfxRenderTarget *rt, r32 clear_color[4], r32 clear_depth, i32 clear_stencil)
{
    g_device_ctx->ClearRenderTargetView(rt->view, clear_color);
    g_device_ctx->OMSetRenderTargets(1, &rt->view, NULL);
    
    if (rt->has_depth)
    {
        g_device_ctx->ClearDepthStencilView(rt->ds_view,  D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 
                                            clear_depth, clear_stencil);
        g_device_ctx->OMSetDepthStencilState(rt->ds_state, 1);
    }
    
    
    D3D11_VIEWPORT Viewport;
    ZeroMemory(&Viewport, sizeof(D3D11_VIEWPORT));
    Viewport.TopLeftX = 0;
    Viewport.TopLeftY = 0;
    Viewport.Width    = (r32)rt->width;
    Viewport.Height   = (r32)rt->height;
    g_device_ctx->RSSetViewports(1, &Viewport);
    
}

void gfx_rt_unbind(GfxRenderTarget *rt)
{
    ID3D11RenderTargetView *tmp = 0;
    g_device_ctx->OMSetRenderTargets(1, &tmp, NULL);
}
