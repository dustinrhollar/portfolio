
struct VertexBuffer
{
    ID3D12Resource          *_handle;
    D3D12_VERTEX_BUFFER_VIEW _buffer_view;
    u64                      _count;
    u64                      _stride;
    
    RenderError Init(u64 vertex_count, u64 stride, void *data);
    RenderError Free();
    
};

struct IndexBuffer
{
    ID3D12Resource          *_handle;
    D3D12_INDEX_BUFFER_VIEW _buffer_view;
    size_t                  _count;
    DXGI_FORMAT             _format;
    
    
    RenderError Init(u64 indices_count, u64 stride, void *data);
    RenderError Free();
    
};



RenderError 
VertexBuffer::Init(u64 vertex_count, u64 stride, void *data)
{
    RenderError result = RenderError::Success;
    D3D12_RANGE read_range{};
    
    u64 size = vertex_count * stride;
    
    // NOTE(Dustin): It's probably not very efficient to create vertex buffers with
    // type "UPLOAD". Wouldn't "DEFAULT" be better with an interim buffer? 
    D3D12_HEAP_PROPERTIES heap_props = {};
    heap_props.Type                  = D3D12_HEAP_TYPE_UPLOAD;
    heap_props.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_props.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
    heap_props.CreationNodeMask      = 1;
    heap_props.VisibleNodeMask       = 1;
    
    D3D12_RESOURCE_DESC rsrc_desc = {};
    rsrc_desc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    rsrc_desc.Alignment           = 0;
    rsrc_desc.Width               = size;
    rsrc_desc.Height              = 1;
    rsrc_desc.DepthOrArraySize    = 1;
    rsrc_desc.MipLevels           = 1;
    rsrc_desc.Format              = DXGI_FORMAT_UNKNOWN;
    rsrc_desc.SampleDesc.Count    = 1;
    rsrc_desc.SampleDesc.Quality  = 0;
    rsrc_desc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    rsrc_desc.Flags               = D3D12_RESOURCE_FLAG_NONE;
    
    JMP_FAILED(d3d_device->CreateCommittedResource(&heap_props,
                                                   D3D12_HEAP_FLAG_NONE,
                                                   &rsrc_desc,
                                                   D3D12_RESOURCE_STATE_GENERIC_READ,
                                                   NULL,
                                                   IIDE(&_handle)),
               RenderError::BufferResourceCreateError);
    
    u8 *data_begin;
    
    read_range.Begin = 0;
    read_range.End = 0;
    
    JMP_FAILED(_handle->Map(0, &read_range, (void**)(&data_begin)), RenderError::ResourceMapError);
    {
        memcpy(data_begin, data, size);
    }
    _handle->Unmap(0, NULL);
    
    _buffer_view = {};
    _buffer_view.BufferLocation = _handle->GetGPUVirtualAddress();
    _buffer_view.StrideInBytes  = (UINT)stride;
    _buffer_view.SizeInBytes    = (UINT)size;
    
    _count  = vertex_count;
    _stride = stride;
    
    LBL_FAIL:;
    return result;
}

RenderError 
VertexBuffer::Free()
{
    RenderError result = RenderError::Success;
    D3D_RELEASE(_handle);
    return result;
}


RenderError 
IndexBuffer::Init(u64 indices_count, u64 stride, void *data)
{
    RenderError result = RenderError::Success;
    
    D3D12_RANGE ReadRange = {};
    u64 size = indices_count * stride;;
    
    // NOTE(Dustin): It's probably not very efficient to create vertex buffers with
    // type "UPLOAD". Wouldn't "DEFAULT" be better with an interim buffer? 
    D3D12_HEAP_PROPERTIES heap_props = {};
    heap_props.Type                  = D3D12_HEAP_TYPE_UPLOAD;
    heap_props.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_props.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
    heap_props.CreationNodeMask      = 1;
    heap_props.VisibleNodeMask       = 1;
    
    D3D12_RESOURCE_DESC rsrc_desc = {};
    rsrc_desc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    rsrc_desc.Alignment           = 0;
    rsrc_desc.Width               = size;
    rsrc_desc.Height              = 1;
    rsrc_desc.DepthOrArraySize    = 1;
    rsrc_desc.MipLevels           = 1;
    rsrc_desc.Format              = DXGI_FORMAT_UNKNOWN;
    rsrc_desc.SampleDesc.Count    = 1;
    rsrc_desc.SampleDesc.Quality  = 0;
    rsrc_desc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    rsrc_desc.Flags               = D3D12_RESOURCE_FLAG_NONE;
    
    JMP_FAILED(d3d_device->CreateCommittedResource(&heap_props,
                                                   D3D12_HEAP_FLAG_NONE,
                                                   &rsrc_desc,
                                                   D3D12_RESOURCE_STATE_GENERIC_READ,
                                                   NULL,
                                                   IIDE(&_handle)),
               RenderError::BufferResourceCreateError);
    
    u8 *data_begin;
    
    ReadRange.Begin = 0;
    ReadRange.End = 0;
    
    JMP_FAILED(_handle->Map(0, &ReadRange, (void**)(&data_begin)),
               RenderError::ResourceMapError);
    {
        memcpy(data_begin, data, size);
    }
    _handle->Unmap(0, NULL);
    
    _format = (stride == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    _buffer_view = {};
    _buffer_view.BufferLocation = _handle->GetGPUVirtualAddress();
    _buffer_view.Format         = _format;
    _buffer_view.SizeInBytes    = (UINT)size;
    
    LBL_FAIL:;
    return result;
}

RenderError IndexBuffer::Free()
{
    RenderError result = RenderError::Success;
    D3D_RELEASE(_handle);
    return result;
}
