
struct CommandList
{
    ID3D12GraphicsCommandList *handle;
    // NOTE(Dustin): This is a very lazy way of handling needed multiple
    // command allocators for in-flight frames. Future me: find a better approach!
    ID3D12CommandAllocator    *allocators[MAX_BACK_BUFFER_COUNT];
    D3D12_COMMAND_LIST_TYPE    type;
    
    RenderError Init(D3D12_COMMAND_LIST_TYPE list_type);
    RenderError Free();
    
    /* Resets the Command List and Allocator and begins recording */
    RenderError BeginRecording(i32 frame_index);
    RenderError EndRecording();
    
    RenderError TransitionBarrier(ID3D12Resource* resource,
                                  D3D12_RESOURCE_STATES state_before,
                                  D3D12_RESOURCE_STATES state_after,
                                  UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
    
    void SetScissorRect(const D3D12_RECT &scissor_rect);
    void SetScissorRects(const D3D12_RECT *scissor_rect, u32 num_rects);
    
    void SetViewport(const D3D12_VIEWPORT &viewport);
    void SetViewports(const D3D12_VIEWPORT *viewports, u32 num_viewports);
    
    void ClearTexture(D3D12_CPU_DESCRIPTOR_HANDLE texture, const float clear_color[4]);
    void ClearDepthStencilTexture(D3D12_CPU_DESCRIPTOR_HANDLE texture, 
                                  D3D12_CLEAR_FLAGS clear_flags,
                                  r32 depth = 1.0f,
                                  u8 stencil = 0);
    void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE &rtv);
    
    void SetRootSignature(ID3D12RootSignature *root_signature);
    void SetPipelineState(ID3D12PipelineState *pipeline_state);
    void SetTopology(D3D12_PRIMITIVE_TOPOLOGY topology);
    void SetVertexBuffers(UINT                            start_slot,
                          UINT                            num_views,
                          const D3D12_VERTEX_BUFFER_VIEW *views);
    void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW *pView);
    void DrawIndexedInstanced(UINT index_count_per_instance,
                              UINT instance_count,
                              UINT start_index_location = 0,
                              INT  base_vertex_location = 0,
                              UINT start_instance_location = 0);
    
};

struct CommandQueue
{
    ID3D12CommandQueue     *handle;
    D3D12_COMMAND_LIST_TYPE type;
    
    // TODO(Dustin): Synchronization?
    ID3D12Fence            *fence;
    u64                     fence_value;
    HANDLE                  fence_event;
    
    RenderError Init(D3D12_COMMAND_LIST_TYPE queue_type);
    RenderError Free();
    
    RenderError GetCommandList(CommandList *cmd_list);
    RenderError ExecuteCommandLists(ID3D12CommandList **cmd_lists, i32 count);
    
    RenderError Signal(u64 *signal_val);
    bool IsFenceComplete(u64 fence_value);
    RenderError WaitForFenceValue(u64 fence_value);
};

RenderError
CommandQueue::Init(D3D12_COMMAND_LIST_TYPE queue_type)
{
    RenderError result = RenderError::Success;
    
    type = queue_type;
    
    D3D12_COMMAND_QUEUE_DESC queue_desc{};
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.Type  = type;
    JMP_FAILED(d3d_device->CreateCommandQueue(&queue_desc, IIDE(&handle)),
               RenderError::BadCommandQueue);
    
    fence_value = 0;
    JMP_FAILED(d3d_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IIDE(&fence)), RenderError::BadFence);
    fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fence_event && "Failed to create fence event!\n");
    
    LBL_FAIL:;
    return result;
}

RenderError 
CommandQueue::Free()
{
    RenderError result = RenderError::Success;
    D3D_RELEASE(handle);
    return result;
}

RenderError 
CommandQueue::GetCommandList(CommandList *cmd_list)
{
    RenderError result = RenderError::Success;
    result = cmd_list->Init(type);
    return result;
}

RenderError
CommandQueue::ExecuteCommandLists(ID3D12CommandList **cmd_lists, i32 count)
{
    RenderError result = RenderError::Success;
    handle->ExecuteCommandLists(count, cmd_lists);
    
    // NOTE(Dustin): Signal or leave that up to the swapchain?
    // NOTE(Dustin): Could keep track of "In-flight" command lists, but would
    // need to pair command lists to a fence value in order to track when their
    // CommandAllocator has completed. This would require an extra signal call.
    
    return result;
}

RenderError
CommandQueue::Signal(u64 *signal_val)
{
    RenderError result = RenderError::Success;
    
    u64 fence_value_for_signal = ++fence_value;
    JMP_FAILED(handle->Signal(fence, fence_value_for_signal), RenderError::FenceSignalError);
    *signal_val = fence_value_for_signal;
    
    LBL_FAIL:;
    return result;
}

RenderError
CommandQueue::WaitForFenceValue(u64 fence_value)
{
    RenderError result = RenderError::Success;
    
    //if (IsFenceComplete(fence_value))
    if (fence->GetCompletedValue() < fence_value)
    {
        JMP_FAILED(fence->SetEventOnCompletion(fence_value, fence_event),
                   RenderError::EventSetError);
        WaitForSingleObject(fence_event, INFINITE);
    }
    
    LBL_FAIL:;
    return result;
}

RenderError 
CommandList::Init(D3D12_COMMAND_LIST_TYPE list_type)
{
    RenderError result = RenderError::Success;
    
    type = list_type;
    
    
    // Create a command allocator for each back buffer that will be rendered to.
    // NOTE(Dustin): Again, this is not optimal. We probably are creating more
    // allocators than will actually be used...
    for (UINT n = 0; n < MAX_BACK_BUFFER_COUNT; n++)
    {
        JMP_FAILED(d3d_device->CreateCommandAllocator(type, IIDE(&allocators[n])),
                   RenderError::BadCommandAllocator);
    }
    
    JMP_FAILED(d3d_device->CreateCommandList(0, type, allocators[0], nullptr, IIDE(&handle)),
               RenderError::BadCommandList);
    
    result = EndRecording();
    
    LBL_FAIL:;
    return result;
}

RenderError 
CommandList::Free()
{
    RenderError result = RenderError::Success;
    
    for (UINT n = 0; n < MAX_BACK_BUFFER_COUNT; n++)
    {
        D3D_RELEASE(allocators[n]);
    }
    D3D_RELEASE(handle);
    
    return result;
}

RenderError 
CommandList::EndRecording()
{
    RenderError result = RenderError::Success;
    if (FAILED(handle->Close()))
    {
        result = RenderError::CommandListCloseError;
    }
    return result;
}

RenderError
CommandList::BeginRecording(i32 frame_index)
{
    RenderError result = RenderError::Success;
    
    JMP_FAILED(allocators[frame_index]->Reset(),
               RenderError::CommandAllocatorResetError);
    
    JMP_FAILED(handle->Reset(allocators[frame_index], nullptr),
               RenderError::CommandListResetError);
    
    LBL_FAIL:;
    return result;
}

// transition a resource to a particular state.
RenderError
CommandList::TransitionBarrier(ID3D12Resource*       resource,
                               D3D12_RESOURCE_STATES state_before,
                               D3D12_RESOURCE_STATES state_after,
                               UINT                  subresource)
{
    RenderError result = RenderError::Success;
    
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = resource;
    barrier.Transition.StateBefore = state_before;
    barrier.Transition.StateAfter = state_after;
    barrier.Transition.Subresource = subresource;
    handle->ResourceBarrier(1, &barrier);
    
    return result;
}


void 
CommandList::SetScissorRect(const D3D12_RECT &scissor_rect)
{
    SetScissorRects(&scissor_rect, 1);
}

void 
CommandList::SetScissorRects(const D3D12_RECT *scissor_rects, u32 num_rects)
{
    assert(num_rects < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
    handle->RSSetScissorRects((UINT)num_rects, scissor_rects);
}

void 
CommandList::SetViewport(const D3D12_VIEWPORT &viewport)
{
    SetViewports(&viewport, 1);
}

void 
CommandList::SetViewports(const D3D12_VIEWPORT *viewports, u32 num_viewports)
{
    assert(num_viewports < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
    handle->RSSetViewports((UINT)num_viewports, viewports);
}


void 
CommandList::ClearTexture(D3D12_CPU_DESCRIPTOR_HANDLE texture, const float clear_color[4])
{
    // NOTE(Dustin): Assumes the Texture has already been transition to:
    // D3D12_RESOURCE_STATE_RENDER_TARGET
    handle->ClearRenderTargetView(texture, clear_color, 0, NULL);
}

void 
CommandList::ClearDepthStencilTexture(D3D12_CPU_DESCRIPTOR_HANDLE texture, 
                                      D3D12_CLEAR_FLAGS clear_flags,
                                      r32 depth,
                                      u8 stencil)
{
    // NOTE(Dustin): Assumes the Texture has already been transition to:
    // D3D12_RESOURCE_STATE_DEPTH_WRITE
    handle->ClearDepthStencilView(texture, 
                                  clear_flags,
                                  depth,
                                  stencil,
                                  0, nullptr);
}

void 
CommandList::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE &rtv)
{
    // NOTE(Dustin): Assumes the Texture has already been transition to:
    // D3D12_RESOURCE_STATE_RENDER_TARGET
    // TODO(Dustin): Depth-Stencil View
    handle->OMSetRenderTargets(1, &rtv, FALSE, NULL);
}


void 
CommandList::SetRootSignature(ID3D12RootSignature *root_signature)
{
    
    handle->SetGraphicsRootSignature(root_signature);
}

void 
CommandList::SetPipelineState(ID3D12PipelineState *pipeline_state)
{
    handle->SetPipelineState(pipeline_state);
}

void 
CommandList::SetTopology(D3D12_PRIMITIVE_TOPOLOGY topology)
{
    handle->IASetPrimitiveTopology(topology);
}

void 
CommandList::SetVertexBuffers(UINT                            start_slot,
                              UINT                            num_views,
                              const D3D12_VERTEX_BUFFER_VIEW *views)
{
    handle->IASetVertexBuffers(start_slot, num_views, views);
}

void 
CommandList::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW *pView)
{
    handle->IASetIndexBuffer(pView);
}

void 
CommandList::DrawIndexedInstanced(UINT index_count_per_instance,
                                  UINT instance_count,
                                  UINT start_index_location,
                                  INT  base_vertex_location ,
                                  UINT start_instance_location)
{
    handle->DrawIndexedInstanced(index_count_per_instance, 
                                 instance_count, 
                                 start_index_location, 
                                 base_vertex_location, 
                                 start_instance_location);
}
