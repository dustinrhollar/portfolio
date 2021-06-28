#include <stdint.h>
#include <assert.h>
#include <float.h>
#include <stdlib.h> 
#include <stdarg.h>
#include <math.h>
#include <stdio.h>

#if !defined(MemAlloc) || !defined(MemFree)
#define MemAlloc malloc
#define MemFree  free
#endif

#include "Platform/PlatformTypes.h"
#include "Core/Core.h"
#include "Platform/Platform.h"
#include "Platform/HostKey.h"
#include "Platform/HostWindow.h"
#include "Util/MapleMath.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dxgi.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#if defined(_DEBUG)
#include <dxgidebug.h>
#endif

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define IIDE IID_PPV_ARGS

#define JMP_FAILED(f, e)   \
{                      \
HRESULT hr = (f);  \
if (FAILED(hr)) {  \
result = e;    \
goto LBL_FAIL; \
}                  \
}

#define D3D_RELEASE(r) { (r)->Release(); (r) = 0; }

#include "Renderer.h"

static const u32 c_allow_tearing           = 0x1;
static const u32 c_require_tearing_support = 0x2;
static const u64 MAX_BACK_BUFFER_COUNT     = 3;

static UINT           adapter_id_override = UINT_MAX;
static IDXGIAdapter1 *adapter = 0;
static UINT           adapter_id = UINT_MAX;
static char16_t      *adapter_description;
// Direct3D objects.
static ID3D12Device  *d3d_device = 0;

// Swap chain objects.
static IDXGIFactory4   *dxgi_factory = 0;
static ID3D12Resource  *depth_stencil = 0;

// Direct3D rendering objects.
static ID3D12DescriptorHeap *dsv_descriptor_heap = 0;
static D3D12_VIEWPORT        screen_viewport{};
static D3D12_RECT            scissor_rect{};

// Direct3D properties.
static DXGI_FORMAT       back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM;
static DXGI_FORMAT       depth_buffer_format = DXGI_FORMAT_D32_FLOAT;
static UINT              back_buffer_count = 2;
static D3D_FEATURE_LEVEL d3d_min_feature_level = D3D_FEATURE_LEVEL_11_0;

// Cached device properties.
static host_wnd_t        window = 0;
static bool              is_fullscreen = false;
static D3D_FEATURE_LEVEL d3d_feature_level = D3D_FEATURE_LEVEL_11_0;
static RECT              output_size = { 0, 0, 1, 1 };
static bool              is_window_visible = true;

// DeviceResources options (see flags in constructor)
static u32 options = 0;

// Callbacks
static RendererCallbacks g_callbacks{};

// Renderer Source

#include "CommandQueue.cpp"
#include "Swapchain.cpp"
#include "RootSignature.cpp"
#include "Buffers.cpp"
#include "PipelineState.cpp"

static Swapchain           g_swapchain{};      // @INTERNAL
static CommandQueue        g_present_queue{};  // @INTERNAL
static CommandList         g_command_list{};   // @INTERNAL
static RootSignature       g_root_signature{};
static VertexBuffer        g_vertex_buffer{};
static IndexBuffer         g_index_buffer{};
static PipelineStateObject g_pipeline_state{};

// Predfined functions

FORCE_INLINE DXGI_FORMAT NoSRGB(DXGI_FORMAT fmt);
FORCE_INLINE i64 StrLen16(char16_t *strarg);

static RenderError InitializeDXGIAdapter();
static RenderError CreateDeviceResources();
static void HostWndSetWindowZorderToTopMost(host_wnd_t window, bool is_top);
static RenderError CreateWindowSizeDependentResources();
static void WaitForGpu();
static RenderError BeginFrame(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_PRESENT);
static RenderError EndFrame(D3D12_RESOURCE_STATES beforeState = D3D12_RESOURCE_STATE_RENDER_TARGET);
static void HandleDeviceLost();
static RenderError PipelineSetup();

RENDERER_INTERFACE RenderError
RendererInit(RendererInitInfo *info)
{
    RenderError result = RenderError::Success;
    SwapChainConfig config{};
    
    window = info->wnd;
    output_size.left   = output_size.top = 0;
    output_size.right  = info->wnd_width;
    output_size.bottom = info->wnd_height;
    
    g_callbacks = info->callbacks;
    assert(g_callbacks.get_host_wnd_handle && "User must provide get_host_wnd_handle Callback!");
    
    result = InitializeDXGIAdapter();
    if (result != RenderError::Success) goto LBL_EXIT;
    
    result = CreateDeviceResources();
    if (result != RenderError::Success) goto LBL_EXIT;
    
    result = CreateWindowSizeDependentResources();
    if (result != RenderError::Success) goto LBL_EXIT;
    
    result = PipelineSetup();
    if (result != RenderError::Success) goto LBL_EXIT;
    
    HWND hwnd;
    g_callbacks.get_host_wnd_handle((struct HostWndHandle*)&hwnd, window);
    
    config.buffer_count = back_buffer_count;
    config.max_frame_latency = 1;
    config.automatic_rebuild = true;
    if (options & c_allow_tearing)
    {
        // Recommended to always use tearing if supported when using a sync interval of 0.
        // Note this will fail if in true 'fullscreen' mode.
        config.sync_interval = 0;
    }
    else
    {
        // The first argument instructs DXGI to block until VSync, putting the application
        // to sleep until the next VSync. This ensures we don't waste any cycles rendering
        // frames that will never be displayed to the screen.
        config.sync_interval = 1;
    }
    
    result = SwapchainInit(&g_swapchain, hwnd, &g_present_queue, config);
    
    LBL_EXIT:
    return result;
}

RENDERER_INTERFACE RenderError 
RendererFree()
{
    RenderError result = RenderError::Success;
    
    result = SwapchainFree(&g_swapchain);
    result = g_command_list.Free();
    result = g_present_queue.Free();
    D3D_RELEASE(dsv_descriptor_heap);
    D3D_RELEASE(adapter);
    
    return result;
}

RENDERER_INTERFACE RenderError 
RendererEntry(/* TODO(Dustin): CommandList(s) */)
{
    RenderError result = RenderError::Success;
    
    // Setup viewport and whatnot
    HWND hwnd;
    g_callbacks.get_host_wnd_handle((struct HostWndHandle*)&hwnd, window);
    
    RECT rect;
    GetClientRect(hwnd, &rect);
    output_size.left   = output_size.top = 0;
    output_size.right  = rect.right;
    output_size.bottom = rect.bottom;
    
    UINT backBufferWidth  = fast_max(output_size.right - output_size.left, 1);
    UINT backBufferHeight = fast_max(output_size.bottom - output_size.top, 1);
    screen_viewport.TopLeftX = screen_viewport.TopLeftY = 0.f;
    screen_viewport.Width = (r32)backBufferWidth;
    screen_viewport.Height = (r32)backBufferHeight;
    screen_viewport.MinDepth = D3D12_MIN_DEPTH;
    screen_viewport.MaxDepth = D3D12_MAX_DEPTH;
    
    scissor_rect.left = scissor_rect.top = 0;
    scissor_rect.right = backBufferWidth;
    scissor_rect.bottom = backBufferHeight;
    
    // Resize and sync
    result = PrepareFrame(&g_swapchain);
    if (result != RenderError::Success) return result;
    
    // Transition swap image to be drawable
    // and prep command list/allocator
    result = BeginFrame();
    if (result != RenderError::Success) return result;
    
    g_command_list.SetViewport(screen_viewport);
    g_command_list.SetScissorRect(scissor_rect);
    
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = SwapchainGetBackBufferRTVHandle(&g_swapchain);
    g_command_list.SetRenderTarget(rtv);
    
    const r32 clear_color[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    g_command_list.ClearTexture(rtv, clear_color);
    
    g_command_list.SetRootSignature(g_root_signature._handle);
    g_command_list.SetPipelineState(g_pipeline_state._handle);
    g_command_list.SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    g_command_list.SetVertexBuffers(0, 1, &g_vertex_buffer._buffer_view);
    g_command_list.SetIndexBuffer(&g_index_buffer._buffer_view);
    g_command_list.DrawIndexedInstanced(3, 1);
    
    // Transition the render tager back to presentable
    // and close command list recording
    result = EndFrame();
    if (result != RenderError::Success) goto LBL_FAIL;
    
    result = PresentFrame(&g_swapchain);
    if (result != RenderError::Success) return result;
    
    LBL_FAIL:;
    return result;
}

static RenderError
BeginFrame(D3D12_RESOURCE_STATES before_state)
{
    RenderError result = RenderError::Success;
    u32 back_buffer_index = g_swapchain.frame_index;
    
    result = g_command_list.BeginRecording(back_buffer_index);
    if (result != RenderError::Success) goto LBL_FAIL;
    
    result = g_command_list.TransitionBarrier(SwapchainGetCurrentBackBuffer(&g_swapchain),
                                              before_state,
                                              D3D12_RESOURCE_STATE_RENDER_TARGET);
    
    LBL_FAIL:;
    return result;
}

static RenderError 
EndFrame(D3D12_RESOURCE_STATES before_state)
{
    RenderError result = RenderError::Success;
    
    result = g_command_list.TransitionBarrier(SwapchainGetCurrentBackBuffer(&g_swapchain),
                                              before_state,
                                              D3D12_RESOURCE_STATE_PRESENT);
    if (result != RenderError::Success) goto LBL_FAIL;
    
    
    result = g_command_list.EndRecording();
    if (result != RenderError::Success) goto LBL_FAIL;
    
    ID3D12CommandList *commandLists[] = { g_command_list.handle }; 
    g_present_queue.ExecuteCommandLists(commandLists, ARRAYSIZE(commandLists));
    
    LBL_FAIL:;
    return result;
}

FORCE_INLINE i64 
StrLen16(char16_t *strarg)
{
    if(!strarg)
        return -1; //strarg is NULL pointer
    char16_t* str = strarg;
    for(;*str;++str) ; // empty body
    return str-strarg;
}

static RenderError 
InitializeDXGIAdapter()
{
    RenderError result = RenderError::Success;
    bool debugDXGI = false;
    
#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ID3D12Debug *debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IIDE(&debugController))))
        {
            debugController->EnableDebugLayer();
        }
        else
        {
            // Direct3D Debug Device is not available
            result = RenderError::DebugDeviceUnavailable;
        }
        
        IDXGIInfoQueue *dxgiInfoQueue;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IIDE(&dxgiInfoQueue))))
        {
            debugDXGI = true;
            
            if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IIDE(&dxgi_factory))))
            {
                result = RenderError::BadAdapter;
                goto LBL_FAIL;
            }
            
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
            dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
        }
    }
#endif
    
    if (!debugDXGI)
    {
        if (FAILED(CreateDXGIFactory1(IIDE(&dxgi_factory))))
        {
            result = RenderError::BadAdapter;
            goto LBL_FAIL;
        }
    }
    
    // Determines whether tearing support is available for fullscreen borderless windows.
    if (options & (c_allow_tearing | c_require_tearing_support))
    {
        BOOL allowTearing = FALSE;
        
        IDXGIFactory5 *factory5;
        HRESULT hr = dxgi_factory->QueryInterface(IIDE(&factory5));
        
        if (SUCCEEDED(hr))
        {
            hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        }
        
        if (FAILED(hr) || !allowTearing)
        {
            //mprinte("WARNING: Variable refresh rate displays are not supported.\n");
            if (options & c_require_tearing_support)
            {
                result = RenderError::TearingUnavailable;
            }
            options &= ~c_allow_tearing;
        }
    }
    
    // Initialize DXGI Adapter
    
    adapter = nullptr;
    
    IDXGIAdapter1 *padapter;
    IDXGIFactory6 *factory6;
    HRESULT hr = dxgi_factory->QueryInterface(IIDE(&factory6));
    if (FAILED(hr))
    {
        result = RenderError::DXGI_1_6_NotSupported;
        goto LBL_FAIL;
    }
    
    for (UINT adapterID = 0; DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(adapterID, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IIDE(&padapter)); ++adapterID)
    {
        if (adapter_id_override != UINT_MAX && adapterID != adapter_id_override)
        {
            continue;
        }
        
        DXGI_ADAPTER_DESC1 desc;
        if (FAILED(padapter->GetDesc1(&desc)))
        {
            result = RenderError::BadAdapter;
            goto LBL_FAIL;
        }
        
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Don't select the Basic Render Driver adapter.
            continue;
        }
        
        // Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(padapter, d3d_min_feature_level, _uuidof(ID3D12Device), nullptr)))
        {
            adapter_id = adapterID;
            
            i64 len = StrLen16((char16_t*)desc.Description);
            
            adapter_description = (char16_t*)MemAlloc(sizeof(char16_t) * (u32)len+1);
            memcpy(adapter_description, desc.Description, len * sizeof(char16_t));
            adapter_description[len] = 0;
            
#ifdef _DEBUG
            //mprint("Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n", adapterID, desc.VendorId, desc.DeviceId, desc.Description);
#endif
            break;
        }
    }
    
#if !defined(NDEBUG)
    if (!padapter && adapter_id_override == UINT_MAX)
    {
        // Try WARP12 instead
        if (FAILED(dxgi_factory->EnumWarpAdapter(IID_PPV_ARGS(&padapter))))
        {
            //PlatformFatalError("WARP12 not available. Enable the 'Graphics Tools' optional feature");
            result = RenderError::WARPNotAvailable;
            goto LBL_FAIL;
        }
        
        //mprint("Direct3D Adapter - WARP12\n");
    }
#endif
    
    if (!padapter)
    {
        // Leave the if-else for posterity...
        if (adapter_id_override != UINT_MAX)
        {
            //PlatformFatalError("Unavailable adapter requested.");
            result = RenderError::BadAdapter;
        }
        else
        {
            //PlatformFatalError("Unavailable adapter.");
            result = RenderError::BadAdapter;
        }
        goto LBL_FAIL;
    }
    
    adapter = padapter;
    
    LBL_FAIL:;
    return result;
}

FORCE_INLINE DXGI_FORMAT 
NoSRGB(DXGI_FORMAT fmt)
{
    switch (fmt)
    {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return DXGI_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8A8_UNORM;
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8X8_UNORM;
        default:                                return fmt;
    }
}

static RenderError 
CreateDeviceResources()
{
    RenderError result = RenderError::Success;
    
    // Determine maximum supported feature level for this device
    static const D3D_FEATURE_LEVEL s_featureLevels[] =
    {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    
    D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels =
    {
        _countof(s_featureLevels), s_featureLevels, D3D_FEATURE_LEVEL_11_0
    };
    
    D3D12_COMMAND_QUEUE_DESC   queueDesc = {};
    //D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
    D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
    
    // Create the DX12 API device object.
    if (FAILED(D3D12CreateDevice(adapter, d3d_min_feature_level, IIDE(&d3d_device))))
    {
        result = RenderError::BadDevice;
        goto LBL_FAIL;
    }
    
    // ...Not really sure what I was thinking with this block of code.
#ifndef NDEBUG
    // Configure debug device (if active).
    ID3D12InfoQueue *d3dInfoQueue;
    if (SUCCEEDED(d3d_device->QueryInterface(IIDE(&d3dInfoQueue))))
    {
#ifdef _DEBUG
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif
        D3D12_MESSAGE_ID hide[] =
        {
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
        };
        D3D12_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = _countof(hide);
        filter.DenyList.pIDList = hide;
        d3dInfoQueue->AddStorageFilterEntries(&filter);
    }
#endif
    
    HRESULT hr = d3d_device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels));
    if (SUCCEEDED(hr))
    {
        d3d_feature_level = featLevels.MaxSupportedFeatureLevel;
    }
    else
    {
        d3d_feature_level = d3d_min_feature_level;
    }
    
    // Create the command queue.
    result = g_present_queue.Init(D3D12_COMMAND_LIST_TYPE_DIRECT);
    if (result != RenderError::Success) goto LBL_FAIL;
    
    result = g_present_queue.GetCommandList(&g_command_list);
    if (result != RenderError::Success) goto LBL_FAIL;
    
    // Init the depth stencil RTV
    if (depth_buffer_format != DXGI_FORMAT_UNKNOWN)
    {
        dsvDescriptorHeapDesc.NumDescriptors = 1;
        dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        
        if (FAILED(d3d_device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IIDE(&dsv_descriptor_heap))))
        {
            result = RenderError::BadDescriptorHeap;
            goto LBL_FAIL;
        }
    }
    
    LBL_FAIL:;
    return result;
}

static void 
HandleDeviceLost()
{
    g_present_queue.Free();
    g_command_list.Free();
    SwapchainFree(&g_swapchain);
    
    depth_stencil->Release();
    dsv_descriptor_heap->Release();
    d3d_device->Release();
    dxgi_factory->Release();
    adapter->Release();
    
#ifdef _DEBUG
    {
        IDXGIDebug1 *dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
        {
            dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
        }
    }
#endif
    
    InitializeDXGIAdapter();
    CreateDeviceResources();
    CreateWindowSizeDependentResources();
}

static void 
HostWndSetWindowZorderToTopMost(host_wnd_t window, bool is_top)
{
    HWND handle;
    g_callbacks.get_host_wnd_handle((struct HostWndHandle*)&handle, window);
    
    RECT rect;
    GetWindowRect(handle, &rect);
    
    SetWindowPos(handle,
                 (is_top) ? HWND_TOPMOST : HWND_NOTOPMOST,
                 rect.left,
                 rect.top,
                 rect.right - rect.left,
                 rect.bottom - rect.top,
                 SWP_FRAMECHANGED | SWP_NOACTIVATE);
}

static RenderError 
CreateWindowSizeDependentResources()
{
    RenderError result = RenderError::Success;
    
    // Wait until all previous GPU work is complete.
    //WaitForGpu();
    
    // Determine the render target size in pixels.
    UINT backBufferWidth  = fast_max(output_size.right - output_size.left, 1);
    UINT backBufferHeight = fast_max(output_size.bottom - output_size.top, 1);
    DXGI_FORMAT backBufferFormat = NoSRGB(back_buffer_format);
    
    if (depth_buffer_format != DXGI_FORMAT_UNKNOWN)
    {
        // Allocate a 2-D surface as the depth/stencil buffer and create a depth/stencil view
        // on this surface.
        D3D12_HEAP_PROPERTIES dh_prop{};
        dh_prop.Type                 = D3D12_HEAP_TYPE_DEFAULT;
        dh_prop.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        dh_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        dh_prop.CreationNodeMask     = 1;
        dh_prop.VisibleNodeMask      = 1;
        
        D3D12_RESOURCE_DESC ds_desc{};
        ds_desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        ds_desc.Alignment          = 0;
        ds_desc.Width              = backBufferWidth;
        ds_desc.Height             = backBufferHeight;
        ds_desc.DepthOrArraySize   = 1;
        ds_desc.MipLevels          = 1;
        ds_desc.Format             = depth_buffer_format;
        ds_desc.SampleDesc.Count   = 1;
        ds_desc.SampleDesc.Quality = 0;
        ds_desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        ds_desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        
        D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
        depthOptimizedClearValue.Format = depth_buffer_format;
        depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
        depthOptimizedClearValue.DepthStencil.Stencil = 0;
        
        if (FAILED(d3d_device->CreateCommittedResource(&dh_prop,
                                                       D3D12_HEAP_FLAG_NONE,
                                                       &ds_desc,
                                                       D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                       &depthOptimizedClearValue,
                                                       IIDE(&depth_stencil))))
        {
            // Failed to create Committed Resource for DSV!
            result = RenderError::BadRenderTargetView;
            goto LBL_FAIL;
        }
        
        depth_stencil->SetName(L"Depth stencil");
        
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = depth_buffer_format;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        
        d3d_device->CreateDepthStencilView(depth_stencil, 
                                           &dsvDesc, 
                                           dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());
    }
    
    // Set the 3D rendering viewport and scissor rectangle to target the entire window.
    screen_viewport.TopLeftX = screen_viewport.TopLeftY = 0.f;
    screen_viewport.Width = (r32)backBufferWidth;
    screen_viewport.Height = (r32)backBufferHeight;
    screen_viewport.MinDepth = D3D12_MIN_DEPTH;
    screen_viewport.MaxDepth = D3D12_MAX_DEPTH;
    
    scissor_rect.left = scissor_rect.top = 0;
    scissor_rect.right = backBufferWidth;
    scissor_rect.bottom = backBufferHeight;
    
    LBL_FAIL:
    return result;
}

#if 0 
static void 
WaitForGpu()
{
    if (command_queue && fence && fence_event != INVALID_HANDLE_VALUE)
    {
        // Schedule a Signal command in the GPU queue.
        UINT64 fenceValue = fence_values[back_buffer_index];
        if (SUCCEEDED(command_queue->Signal(fence, fenceValue)))
        {
            // Wait until the Signal has been processed.
            if (SUCCEEDED(fence->SetEventOnCompletion(fenceValue, fence_event)))
            {
                WaitForSingleObjectEx(fence_event, INFINITE, FALSE);
                // Increment the fence value for the current frame.
                fence_values[back_buffer_index]++;
            }
        }
    }
}
#endif

static RenderError 
PipelineSetup()
{
    RenderError result = RenderError::Success;
    
    // Create the root signature
    result = g_root_signature.Init(0, 0, 0, 0, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    if (result != RenderError::Success) return result;
    
    // Create Vertex Buffer
    {
        struct Vertex
        {
            v3 Position;
            v3 Color;
        };
        
        Vertex vertices[] = {
            { {  1.0f, -1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
            { { -1.0f, -1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
            { {  0.0f,  1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
        };
        
        result = g_vertex_buffer.Init(_countof(vertices), sizeof(Vertex), (void*)vertices);
        if (result != RenderError::Success) goto LBL_FAIL;
    }
    
    {
        // Index Buffer
        u32 Indices[3] = { 0, 1, 2 };
        const u32 IndexBufferSize = sizeof(Indices);
        g_index_buffer.Init(_countof(Indices), sizeof(u32), (void*)Indices);
    }
    
    // Pipeline
    {
        GfxShaderModules shader_modules;
        result = LoadShaderModules(&shader_modules, 2, 
                                   GfxShaderStage::Vertex, L"shaders/simple_vert.cso",
                                   GfxShaderStage::Pixel,  L"shaders/simple_frag.cso");
        if (result != RenderError::Success) goto LBL_FAIL;
        
        GfxRasterDesc raster_desc = GetDefaultRasterDesc();
        
        GfxInputElementDesc input_desc[] = {
            { "POSITION", 0, GfxFormat::R32G32B32_Float, 0, 0,  GfxInputClass::PerVertex, 0 },
            { "COLOR",    0, GfxFormat::R32G32B32_Float, 0, 12, GfxInputClass::PerVertex, 0 }
        };
        
        
        GfxPipelineStateDesc pso_desc{};
        pso_desc.root_signature = &g_root_signature;
        pso_desc.shader_modules = shader_modules;
        pso_desc.raster = raster_desc;
        pso_desc.input_layouts = input_desc;
        pso_desc.input_layouts_count = _countof(input_desc);
        pso_desc.topology = GfxTopology::Triangle;
        pso_desc.render_target_count = 1;
        pso_desc.rtv_formats[0] = GfxFormat::Swapchain;
        pso_desc.sample_desc.count = 1;
        result = g_pipeline_state.Init(&pso_desc);
    }
    
    LBL_FAIL:;
    return result;
}