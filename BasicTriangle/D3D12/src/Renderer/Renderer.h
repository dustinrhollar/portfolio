#ifndef _RENDERER_H
#define _RENDERER_H

#ifdef _RENDERER_EXPORT
#define RENDERER_INTERFACE extern "C" _declspec(dllexport)
#else
#define RENDERER_INTERFACE extern "C" _declspec(dllimport)
#endif

enum class RenderError : u8
{
    Success,
    DebugDeviceUnavailable,
    TearingUnavailable,
    DXGI_1_5_NotSupported,
    DXGI_1_6_NotSupported,
    BadAdapter,
    WARPNotAvailable,
    BadDevice,
    BadCommandQueue,
    BadDescriptorHeap,
    BadCommandAllocator,
    BadCommandList,
    CommandListCloseError,
    BadFence,
    BadEvent,
    BadRenderTargetView,
    SwapchainPresentError,
    FenceSignalError,
    EventSetError,
    
    BadSwapchain,       // Failed to initialize or get the swapchain
    BadSwapchainBuffer, // Failed to acquire backbuffer for a swapchain
    SwapchainNotReady,  // Set when fence for swapchain is not ready for presentation.
    
    CommandAllocatorResetError,
    CommandListResetError,
    
    BufferResourceCreateError,
    
    RootSignatureError,
    
    ResourceMapError,
    
    ShaderBlobError,
    
    PipelineStateError,
    
    Count,
    Unknown = Count,
};


typedef void (*GetHostWndHandle)(struct HostWndHandle *handle, struct HostWnd *wnd);

struct RendererCallbacks
{
    GetHostWndHandle get_host_wnd_handle;
};

struct RendererInitInfo
{
    struct HostWnd   *wnd;
    i32               wnd_width;
    i32               wnd_height;
    
    RendererCallbacks callbacks;
};

RENDERER_INTERFACE RenderError RendererInit(RendererInitInfo *info);
RENDERER_INTERFACE RenderError RendererFree();
RENDERER_INTERFACE RenderError RendererEntry(/* TODO(Dustin): CommandList(s) */);

#endif //_RENDERER_H
