
//~ Header

// TODO(Matt): Change "automatic_rebuild" to be "manual_rebuild". We'd like the default (0 or false)
// value to be the most common use case. Most of the time automatic rebuild should be enabled.
struct SwapChainConfig
{
    // Total number of buffers. Must be between 2 and DXGI_MAX_SWAP_CHAIN_BUFFERS.
    u32 buffer_count;
    
    // Max number of queued frames. Must be at least 1.
    u32 max_frame_latency;
    
    // VSync interval, must be between 0 and 4. A value of 0 disables VSync, and 1 enables it.
    // Set to 2, 3, or 4 to sync at 1/2, 1/3, or 1/4 the refresh rate.
    u32 sync_interval;
    
    // If true, swap chain will detect window size, buffer count, or max frame latency changes, and
    // resize automatically in BeginFrame(). If false, you must rebuild manually.
    bool automatic_rebuild;
};

struct Swapchain
{
    // Current config. NOTE(Matt): If using automatic rebuild, changing the config can force a
    // resize or rebuild during the next BeginFrame() call, incurring a GPU flush.
    // Changing the buffer count will cause a resize, and changing the max frame latency will force
    // a full rebuild.
    SwapChainConfig config;
    
#if 0
    // Queue used for synchronizing Present() calls.
    ID3D12CommandQueue* present_queue;
#else
    CommandQueue *present_queue;
#endif
    
    // We use an IDXGISwapChain3 because we need to be able to call GetCurrentBackBufferIndex().
    IDXGISwapChain3* handle;
    
    // Index of the current back buffer. This gets updated during every call to BeginFrame().
    u32 frame_index;
    
#if 0
    // GPU fence used to track frame completion. The current fence value represents the last
    // signalled value. If you wanted to wait for all frames to complete their GPU work, you would
    // wait until the "last completed value" equals the current fence value.
    ID3D12Fence* present_fence;
    u64 current_fence_value;
#endif
    
    // Heap used for RTV descriptors. This is initialized to DXGI_MAX_SWAP_CHAIN_BUFFERS in size,
    // so that we do not need to recreate the heap if the buffer count changes.
    ID3D12DescriptorHeap* rtv_heap;
    
    // Per-buffer resources, including the render target and last fence signal value for each back
    // buffer. To check if a back buffer is in use, you can compare its fence value to the "last
    // completed value" of the fence.
    ID3D12Resource* render_targets[DXGI_MAX_SWAP_CHAIN_BUFFERS];
    u64 fence_values[DXGI_MAX_SWAP_CHAIN_BUFFERS];
    
    // Waitable handles. We use a waitable swap chain in order to limit the number of queued frames,
    // and we can wait on these handles to ensure that we don't use or free resourcese that are in
    // use by the GPU, and to defer updating/rendering until we are sure we will have a buffer to
    // render into.
#if 0
    HANDLE fence_complete_event;
#endif
    HANDLE buffer_available_event;
};

/*
Initializes the swap chain, tying it to a platform window. It's usually a good idea to enable
automatic rebuild. If you're unsure, a buffer count of 3 and maximum frame latency of 2 is a
reasonable config for most applications.
*/
static RenderError SwapchainInit(Swapchain *swapchain, HWND hwnd, CommandQueue *present_queue, SwapChainConfig config);
static RenderError SwapchainFree(Swapchain *swapchain);

/*
Call at the beginning of every frame, ideally before game update or input handling.
If wait_for_buffer is true, this will block until we have a buffer available to render into.
This reduces latency in cases where the present queue is full (GPU bound). You can ignore the
return value in this case (it will always be true).

If wait_for_buffer is false, this function will not block, and the return value indicates
 whether there is an available buffer or not. A common use for this is to render into an
off-screen buffer, and only present if there is a swapchain buffer available. This gives
a properly unlocked framerate (for benchmarks or whatever), but offers no reduction in display
latency, and the computer will be going at full speed for no reason.
*/
static RenderError PrepareFrame(Swapchain *swapchain, bool wait_for_buffer = true);

/*
Presents the current frame, increments the current fence value, and signals the command queue
with that new value. The new value is also cached in the array of per-buffer fence values.
If configured with sync interval 0, VSync is off, and the "allow tearing" flag will be used
if available. Otherwise, the "allow tearing" flag is unused, and the configured sync interval
is used for VSync. Note that for most typical configs, if you are allowing BeginFrame() to
block, the call to Present() shouldn't have to.
*/
static RenderError PresentFrame(Swapchain *swapchain);

/*
These two methods get the back buffer and RTV handle for the current frame. Mostly useful for
command list recording, such as inserting a transition barrier or drawing to the back buffer.
It is only valid to call these somewhere between BeginFrame() and PresentFrame(). 
*/
static ID3D12Resource* SwapchainGetCurrentBackBuffer(Swapchain *swapchain);
static D3D12_CPU_DESCRIPTOR_HANDLE SwapchainGetBackBufferRTVHandle(Swapchain *swapchain);


/*
Resize the swap chain. If automatic rebuild is enabled, you likely don't need to call this
manually, as it will be called during BeginFrame() if needed. If not, you'll want to call
this in response to window size or buffer count changes. If there are any buffers still
in-flight, this will block until they are no longer in use.
*/
static RenderError SwapchainResize(Swapchain *swapchain);

/*
Fully rebuild the swap chain. Just like Resize(), you likely don't need to call this if
automatic rebuild is enabled. It will be called during BeginFrame() if needed. Otherwise,
you should only have to call this if you change the maximum frame latency. Other config changes
should be handled by calling Resize(). If there are any buffers still in-flight, this will
block until they are no longer in use.
*/
static RenderError SwapchainRebuild(Swapchain *swapchain);

//~ Implementation

static RenderError 
SwapchainInit(Swapchain *swapchain, HWND hwnd, CommandQueue* present_queue, SwapChainConfig config)
{
    RenderError result = RenderError::Success;
    D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    
    // Make sure the config is valid.
    assert(config.buffer_count > 1 && config.buffer_count <= DXGI_MAX_SWAP_CHAIN_BUFFERS);
    assert(config.sync_interval <= 4);
    assert(config.max_frame_latency > 0);
    
    swapchain->config = config;
    swapchain->present_queue = present_queue;
    
    // Get the device that the command queue belongs to.
    ID3D12Device* device = 0;
    JMP_FAILED(present_queue->handle->GetDevice(__uuidof(ID3D12Device), (void**)&device), 
               RenderError::BadDevice);
    
    // Create the RTV heap.
    rtv_heap_desc.NumDescriptors = DXGI_MAX_SWAP_CHAIN_BUFFERS;
    rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    JMP_FAILED(device->CreateDescriptorHeap(&rtv_heap_desc, __uuidof(ID3D12DescriptorHeap), (void**)&swapchain->rtv_heap),
               RenderError::BadDescriptorHeap);
    
    // Initialize all the RTVs and fence values to 0.
    for (u32 i = 0; i < DXGI_MAX_SWAP_CHAIN_BUFFERS; ++i)
    {
        swapchain->render_targets[i] = 0;
        swapchain->fence_values[i] = 0;
    }
    //swapchain->current_fence_value = 0;
    
    // Check if we have tearing support. This is almost always true on Windows 10.
    //IDXGIFactory4* factory = CreateDXGIFactory4();
    BOOL allow_tearing = FALSE;
    IDXGIFactory5* factory_5 = 0;
    if (SUCCEEDED(dxgi_factory->QueryInterface(__uuidof(IDXGIFactory5), (void**)&factory_5)))
    {
        if (FAILED(factory_5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing))))
        {
            allow_tearing = FALSE;
        }
    }
    if (factory_5) factory_5->Release();
    
    desc.Width = 0;
    desc.Height = 0;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Stereo = FALSE;
    desc.SampleDesc = { 1, 0 };
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = config.buffer_count;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    u32 flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    if (allow_tearing) flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    desc.Flags = flags;
    IDXGISwapChain1* swap_chain_1 = 0;
    JMP_FAILED(dxgi_factory->CreateSwapChainForHwnd(present_queue->handle, hwnd, &desc, 0, 0, &swap_chain_1),
               RenderError::BadSwapchain);
    
    // Disable the automatic Alt+Enter fullscreen toggle. We'll do this ourselves to support adaptive refresh rate stuff.
    JMP_FAILED(dxgi_factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER),
               RenderError::BadSwapchain);
    
    // Cast to an IDXGISwapChain3.
    JMP_FAILED(swap_chain_1->QueryInterface(_uuidof(IDXGISwapChain3), (void**)&swapchain->handle),
               RenderError::BadSwapchain);
    swap_chain_1->Release();
    //factory->Release();
    
    swapchain->frame_index = swapchain->handle->GetCurrentBackBufferIndex();
    
    u32 descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = swapchain->rtv_heap->GetCPUDescriptorHandleForHeapStart();
    for (u32 i = 0; i < config.buffer_count; ++i)
    {
        JMP_FAILED(swapchain->handle->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&swapchain->render_targets[i]),
                   RenderError::BadRenderTargetView);
        device->CreateRenderTargetView(swapchain->render_targets[i], 0, rtv_handle);
        rtv_handle.ptr += descriptor_size;
    }
    
    swapchain->buffer_available_event = swapchain->handle->GetFrameLatencyWaitableObject();
    
#if 0
    swapchain->current_fence_value = 0;
    JMP_FAILED(device->CreateFence(swapchain->current_fence_value, D3D12_FENCE_FLAG_NONE, 
                                   __uuidof(ID3D12Fence), (void**)&swapchain->present_fence),
               RenderError::BadFence);
    swapchain->fence_complete_event = CreateEvent(0, FALSE, FALSE, 0);
#endif
    
    swapchain->handle->SetMaximumFrameLatency(swapchain->config.max_frame_latency);
    device->Release();
    
    LBL_FAIL:;
    return result;
}

static RenderError 
SwapchainFree(Swapchain *swapchain)
{
    RenderError result = RenderError::Success;
    
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    JMP_FAILED(swapchain->handle->GetDesc1(&desc), RenderError::BadSwapchain);
    for (u32 i = 0; i < desc.BufferCount; ++i)
    {
        D3D_RELEASE(swapchain->render_targets[i]);
    }
    D3D_RELEASE(swapchain->rtv_heap);
    D3D_RELEASE(swapchain->handle);
    //D3D_RELEASE(swapchain->present_fence);
    CloseHandle(swapchain->buffer_available_event);
    //CloseHandle(swapchain->fence_complete_event);
    swapchain->present_queue = 0;
    
    LBL_FAIL:
    return result;
}

static RenderError 
PrepareFrame(Swapchain *swapchain, bool wait_for_buffer)
{
    RenderError result = RenderError::Success;
    
    if (swapchain->config.automatic_rebuild)
    {
        // Check if we need to rebuild or resize.
        RECT rect;
        HWND hwnd;
        DXGI_SWAP_CHAIN_DESC1 desc;
        JMP_FAILED(swapchain->handle->GetHwnd(&hwnd),
                   RenderError::BadSwapchain);
        GetClientRect(hwnd, &rect);
        JMP_FAILED(swapchain->handle->GetDesc1(&desc),
                   RenderError::BadSwapchain);
        
        u32 current_w = desc.Width;
        u32 current_h = desc.Height;
        u32 current_d = desc.BufferCount;
        
        u32 new_w = rect.right;
        u32 new_h = rect.bottom;
        u32 new_d = swapchain->config.buffer_count;
        
        u32 current_latency;
        swapchain->handle->GetMaximumFrameLatency(&current_latency);
        u32 new_latency = swapchain->config.max_frame_latency;
        
        // If the max latency changed, we need to do a full rebuild. Otherwise, we can resize if needed.
        if (new_latency != current_latency)
        {
            SwapchainRebuild(swapchain);
        }
        else if ((new_w != current_w || new_h != current_h || new_d != current_d) && (new_w > 0 && new_h > 0))
        {
            SwapchainResize(swapchain);
        }
    }
    
    swapchain->frame_index = swapchain->handle->GetCurrentBackBufferIndex();
    
    // Make sure our buffer for this frame isn't still in use by the GPU.
    // If the new back buffer isn't available yet, we must wait.
#if 0
    if (swapchain->present_fence->GetCompletedValue() < swapchain->fence_values[swapchain->frame_index])
    {
        JMP_FAILED(swapchain->present_fence->SetEventOnCompletion(swapchain->fence_values[swapchain->frame_index], 
                                                                  swapchain->fence_complete_event),
                   RenderError::EventSetError);
        WaitForSingleObject(swapchain->fence_complete_event, INFINITE);
    }
#else
    result = swapchain->present_queue->WaitForFenceValue(swapchain->fence_values[swapchain->frame_index]);
    assert(result == RenderError::Success);
#endif
    
#if 0
    if (!has_started)
    {
        TimerBegin(&cool_timer);
    }
#endif
    
    if (wait_for_buffer)
    {
        //u64 nanos = Win32MeasureNanoseconds(&cool_timer);
        DWORD wait_result = WaitForSingleObject(swapchain->buffer_available_event, INFINITE);
        assert(wait_result == WAIT_OBJECT_0);
        
        //u64 new_nanos = Win32MeasureNanoseconds(&cool_timer);
        //double secs = NanosecondsToSeconds(new_nanos - nanos);
        //PrintF("%.3fms\n", (float)secs * 1000);
    }
    else
    {
        DWORD wait_result = WaitForSingleObject(swapchain->buffer_available_event, 0);
        if (wait_result == WAIT_TIMEOUT) result = RenderError::SwapchainNotReady;
        assert(wait_result == WAIT_TIMEOUT || wait_result == WAIT_OBJECT_0);
    }
    
    LBL_FAIL:
    return result;
}

static RenderError PresentFrame(Swapchain *swapchain)
{
    RenderError result = RenderError::Success;
    
    u32 flags = 0;
    if (swapchain->config.sync_interval == 0)
    {
        DXGI_SWAP_CHAIN_DESC1 desc = {};
        JMP_FAILED(swapchain->handle->GetDesc1(&desc),
                   RenderError::BadSwapchain);
        if ((desc.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING) != 0) flags |= DXGI_PRESENT_ALLOW_TEARING;
    }
    
    JMP_FAILED(swapchain->handle->Present(swapchain->config.sync_interval, flags),
               RenderError::SwapchainPresentError);
    
    // Schedule a fence signal in the command queue.
#if 0
    swapchain->fence_values[swapchain->frame_index] = ++swapchain->current_fence_value;
    JMP_FAILED(swapchain->present_queue->Signal(swapchain->present_fence, swapchain->current_fence_value),
               RenderError::FenceSignalError);
#else
    result = swapchain->present_queue->Signal(&swapchain->fence_values[swapchain->frame_index]);
#endif
    
    LBL_FAIL:
    return result;
}


static RenderError 
SwapchainResize(Swapchain *swapchain)
{
    RenderError result = RenderError::Success;
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    
    assert(swapchain->config.buffer_count > 1 && 
           swapchain->config.buffer_count <= DXGI_MAX_SWAP_CHAIN_BUFFERS);
    
    // Wait until we can safely resize.
#if 0
    if (swapchain->present_fence->GetCompletedValue() < swapchain->fence_values[swapchain->frame_index])
    {
        JMP_FAILED(swapchain->present_fence->SetEventOnCompletion(swapchain->fence_values[swapchain->frame_index], 
                                                                  swapchain->fence_complete_event),
                   RenderError::EventSetError);
        WaitForSingleObject(swapchain->fence_complete_event, INFINITE);
    }
#else
    result = swapchain->present_queue->WaitForFenceValue(swapchain->fence_values[swapchain->frame_index]);
    assert(result == RenderError::Success);
#endif
    
    // Release old resources.
    JMP_FAILED(swapchain->handle->GetDesc1(&desc),
               RenderError::BadSwapchain);
    for (u32 i = 0; i < desc.BufferCount; ++i) swapchain->render_targets[i]->Release();
    
    // We always want the waitable object flag, and the tearing flag if it is supported.
    u32 flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    if ((desc.Flags & DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING) != 0) flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    JMP_FAILED(swapchain->handle->ResizeBuffers(swapchain->config.buffer_count, 0, 0, DXGI_FORMAT_UNKNOWN, flags),
               RenderError::BadSwapchain);
    
    // Set up the render target views and descriptor handles.
    ID3D12Device* device = 0;
    JMP_FAILED(swapchain->present_queue->handle->GetDevice(__uuidof(ID3D12Device), (void**)&device),
               RenderError::BadDevice);
    u32 descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = swapchain->rtv_heap->GetCPUDescriptorHandleForHeapStart();
    for (u32 i = 0; i < swapchain->config.buffer_count; ++i)
    {
        JMP_FAILED(swapchain->handle->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&swapchain->render_targets[i]),
                   RenderError::BadRenderTargetView);
        device->CreateRenderTargetView(swapchain->render_targets[i], 0, rtv_handle);
        rtv_handle.ptr += descriptor_size;
        swapchain->fence_values[i] = swapchain->present_queue->fence_value;
    }
    device->Release();
    
    LBL_FAIL:;
    return result;
}

static RenderError
SwapchainRebuild(Swapchain *swapchain)
{
    RenderError result = RenderError::Success;
    
    assert(swapchain->config.buffer_count > 1 && swapchain->config.buffer_count <= DXGI_MAX_SWAP_CHAIN_BUFFERS);
    assert(swapchain->config.max_frame_latency > 0);
    
    // Wait until we can safely resize.
#if 0
    if (swapchain->present_fence->GetCompletedValue() < swapchain->fence_values[swapchain->frame_index])
    {
        JMP_FAILED(swapchain->present_fence->SetEventOnCompletion(swapchain->fence_values[swapchain->frame_index], 
                                                                  swapchain->fence_complete_event),
                   RenderError::EventSetError);
        WaitForSingleObject(swapchain->fence_complete_event, INFINITE);
    }
#else
    result = swapchain->present_queue->WaitForFenceValue(swapchain->fence_values[swapchain->frame_index]);
    assert(result == RenderError::Success);
#endif
    
    HWND hwnd;
    swapchain->handle->GetHwnd(&hwnd);
    SwapChainConfig cfg = swapchain->config;
    CommandQueue* queue = swapchain->present_queue;
    
    result = SwapchainFree(swapchain);
    assert(result == RenderError::Success);
    
    result = SwapchainInit(swapchain, hwnd, queue, cfg);
    assert(result == RenderError::Success);
    
    return result;
}

static ID3D12Resource* SwapchainGetCurrentBackBuffer(Swapchain *swapchain)
{
    ID3D12Resource* result = swapchain->render_targets[swapchain->frame_index];
    return result;
}

static D3D12_CPU_DESCRIPTOR_HANDLE SwapchainGetBackBufferRTVHandle(Swapchain *swapchain)
{
    D3D12_CPU_DESCRIPTOR_HANDLE result = swapchain->rtv_heap->GetCPUDescriptorHandleForHeapStart();
    ID3D12Device* device = 0;
    
    HRESULT hr = swapchain->present_queue->handle->GetDevice(__uuidof(ID3D12Device), (void**)&device);
    assert(SUCCEEDED(hr));
    
    u32 rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    result.ptr += swapchain->frame_index * rtv_descriptor_size;
    device->Release();
    return result;
}
