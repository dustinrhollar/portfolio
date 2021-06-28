
#define DEFAULT_WINDOW_WIDTH  1920
#define DEFAULT_WINDOW_HEIGHT 1080
#define WINDOW_NAME           "Maple D3D12 Example"

file_global bool             g_app_is_running = false;
file_global bool             g_needs_resized  = false;
file_global bool             g_fullscreen     = false;
file_global HostWnd         *g_client         = 0;
file_global Win32ThreadPool *g_thread_pool    = 0;

void 
PlatformGetWindowDims(u32 *width, u32 *height)
{
    HostWndGetDims(g_client, width, height);
}

void
Win32KeyPressCallback(MapleKey key)
{
    if (key == Key_Escape)
        g_app_is_running = false;
}

void 
PlatformAsyncTask(void (*fn)(void*), void *args)
{
    Win32ThreadQueueTask(g_thread_pool, fn, args);
}

void 
Win32ResizeCallback(u32 width, u32 height)
{
    // NOTE(Dustin): You would resize the renderer here
}

void 
PlatformAtomicInc(volatile u32* v)
{
    _InterlockedIncrement(v);
}

void 
PlatformAtomicDec(volatile u32* v)
{
    _InterlockedDecrement(v);
}

struct HostWndHandle { HWND handle; };

static void
Win32GetHostWindowHandle(HostWndHandle *wnd_handle, HostWnd *wnd)
{
    wnd_handle->handle = wnd->handle;
}

INT WINAPI 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    GlobalTimerSetup();
    PlatformLoggerInit();
    
    //~ initialze memory for the application
    
    u64   app_backing_memory_size = _MB(512);
    void *app_backing_memory = PlatformAlloc(app_backing_memory_size);
    SysMemoryInit(app_backing_memory, app_backing_memory_size);
    
    Win32ProcessorInfo processor_info;
    Win32GetProcessorInfo(&processor_info);
    Win32ThreadPoolInit(&g_thread_pool, processor_info.logical_processor_count, 500);
    
    //~ Create the g_client window
    
    HostWndInit(&g_client, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, WINDOW_NAME);
    
    HostWndCallbacks callbacks = {0};
    callbacks.press  = Win32KeyPressCallback;
    callbacks.resize = Win32ResizeCallback;
    HostWndSetCallbacks(g_client, &callbacks);
    
    //~ Initialize the renderer
    
    u32 width, height;
    PlatformGetWindowDims(&width, &height);
    
    RendererCallbacks renderer_callbacks{};
    renderer_callbacks.get_host_wnd_handle = Win32GetHostWindowHandle;
    
    RendererInitInfo render_info{};
    render_info.wnd = g_client;
    render_info.wnd_width = width;
    render_info.wnd_height = height;
    render_info.callbacks = renderer_callbacks;
    
    RenderError res = RendererInit(&render_info);
    Assert(res == RenderError::Success);
    
    //~ BEGIN!
    
    r32 RefreshRate = 60.0f;
    r32 TargetSecondsPerFrame = 1 / RefreshRate;
    u64 frame_counter = 0;
    b8 last_frame_render = 1;
    Timer frame_timer;
    
    HostWndSetActive(g_client);
    g_app_is_running = true;
    TimerBegin(&frame_timer);
    
    while (g_app_is_running) 
    {
        // Handle all outstanding window messages.
        
        if (!HostWndMsgLoop(g_client))
        {
            g_app_is_running = false;
            continue;
        }
        
        // Resize, if needed 
        
        if (g_fullscreen)
        {
            // TODO(Dustin):
        }
        
        if (g_needs_resized)
        {
            g_needs_resized = false;
        }
        
        //~ Render/Game Logic/etc.
        
        RenderError render_err = RendererEntry();
        Assert(render_err == RenderError::Success);
        
        //~ Meet frame rate, if necessary
        
        r32 time_elapsed = TimerMiliSecondsElapsed(&frame_timer);
        
        r32 SecondsElapsedUpdate = TimerSecondsElapsed(&frame_timer);
        
        r32 SecondsElapsedPerFrame = SecondsElapsedUpdate;
        if (SecondsElapsedPerFrame < TargetSecondsPerFrame)
        {
            DWORD SleepMs = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecondsElapsedPerFrame));
            if (SleepMs > 0)
            {
                Sleep(SleepMs);
            }
            SecondsElapsedPerFrame = TimerSecondsElapsed(&frame_timer);
        }
        else
        {
            // LOG: Missed frame rate!
        }
        
        TimerBegin(&frame_timer);
    }
    
    res = RendererFree();
    Assert(res == RenderError::Success);
    
    Win32ThreadPoolFree(&g_thread_pool);
    HostWndFree(&g_client);
    SysMemoryFree();
    PlatformFree(app_backing_memory);
    PlatformLoggerFree();
    
    return (0);
}
