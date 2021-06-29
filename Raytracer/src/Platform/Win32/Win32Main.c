
enum class Mode : u8
{
    Offline,
    OnlineBlocking, // present only when finished
    OnlineAsync,    // update as it renders
    OnlineAsyncJob, // Image is split into "jobs"
};

file_global Mode g_rt_mode = Mode::OnlineAsyncJob;

#define DEFAULT_WINDOW_WIDTH  640
#define DEFAULT_WINDOW_HEIGHT 360
//#define TEXTURE_WIDTH  640
//#define TEXTURE_HEIGHT 360
#define TEXTURE_WIDTH  1920
#define TEXTURE_HEIGHT 1080
#define WINDOW_NAME           "Maple Raytracer"
#define MAPLE_STARTUP_FILE    "maple.startup"

file_global bool g_app_is_running = false;
file_global bool g_needs_resized  = false;
file_global bool g_fullscreen     = false;
file_global HostWnd *g_client = 0;
file_global Win32ThreadPool *g_thread_pool = 0;

file_global r32 g_clear_color[4] = {24.0f/255.f, 0.0f, 0.0f/255.f, 1.0f};

file_global HANDLE rt_async_simple;
volatile u32 g_render_active = 0;

// For async job
file_global RtJob *job_data = 0;
file_global Timer  rt_timer;

void 
PlatformGetWindowDims(u32 *width, u32 *height)
{
    host_wnd_get_dims(g_client, width, height);
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
    renderer_resize(width, height);
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


DWORD WINAPI 
rt_online_async_proc(LPVOID lp_param)
{
    _InterlockedExchange(&g_render_active, 1);
    rt_entry((RaytracerSettings*)lp_param);
    _InterlockedExchange(&g_render_active, 0);
    return 0;
}

DWORD WINAPI 
rt_online_async_job_proc(LPVOID lp_param)
{
    RtJob *job = (RtJob*)lp_param;
    return 0;
}

file_internal void
rt_async_setup_jobs(i32 scan_x, i32 scan_y, RaytracerSettings *settings)
{
    // Think of the image as a linear array of pixels
    // scan_x * scan_y is the number of pixels to process
    Assert(settings->width % scan_x == 0 && settings->height % scan_y == 0);
    u32 job_cols = settings->width / scan_x;
    u32 job_rows = settings->height / scan_y;
    
    u32 job_count = (settings->width / scan_x) * (settings->height / scan_y);
    for (u32 j = 0; j < job_rows; ++j)
    {
        for (u32 i = 0; i < job_cols; ++i)
        {
            i32 real_row = j * scan_y;
            i32 adj_row  = (settings->height - 1) - real_row;
            i32 real_col = i * scan_x;
            
            RtJob job{};
            job.settings = settings;
            job.dims[0]  = scan_x;
            job.dims[1]  = scan_y;
            job.pos[0]   = real_col;
            job.pos[1]   = adj_row;
            job.counter  = &g_render_active;
            arrput(job_data, job);
        }
    }
    
    // Submit the jobs
    _InterlockedExchange(&g_render_active, job_count);
    timer_begin(&rt_timer);
    for (i32 i = 0; i < arrlen(job_data); ++i)
    {
        PlatformAsyncTask(rt_async, &job_data[i]);
    }
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    global_timer_setup();
    PlatformLoggerInit();
    
    //~ initialze memory for the application
    
    u64   app_backing_memory_size = _MB(512);
    void *app_backing_memory = PlatformAlloc(app_backing_memory_size);
    SysMemoryInit(app_backing_memory, app_backing_memory_size);
    
    Win32ProcessorInfo processor_info;
    Win32GetProcessorInfo(&processor_info);
    Win32ThreadPoolInit(&g_thread_pool, processor_info.logical_processor_count, 500);
    
    //~ Create the g_client window
    
    host_wnd_init(&g_client, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "Maple Terrain");
    
    HostWndCallbacks callbacks = {0};
    callbacks.press  = Win32KeyPressCallback;
    callbacks.resize = Win32ResizeCallback;
    host_wnd_set_callbacks(g_client, &callbacks);
    
    //~ Init rest of stuff
    
    u32 width, height;
    host_wnd_get_dims(g_client, &width, &height);
    renderer_init(width, height, 60, 1, g_client);
    
    //~ Camera
    
    const r32 aspect_ratio = 16.0f / 9.0f;
    
    r32 viewport_height = 2.0f;
    r32 viewport_width = aspect_ratio * viewport_height;
    r32 focal_length = 1.0f;
    
    // NOTE(Dustin): Harcoding the camera for now...
    
    CameraCreateInfo info{};
    info.look_from    = { 13, 2, 3 };
    info.look_at      = V3_ZERO;
    info.up           = { 0, 1, 0 };
    info.vfov         = 20;
    info.aspect_ratio = aspect_ratio;
    info.aperture     = 0.1f;
    info.focus_dist   = 10.0f;
    info.t0           = 0.0f;
    info.t1           = 1.0f;
    
    Camera camera;
    camera_init(&camera, &info);
    
    //~ Create the scene
    
    Scene scene;
    scene_init(&scene, 100);
    build_random_scene(&scene, false);
    
    // Build a bvh tree for the scene
    Primitive bvh_tree;
    make_bvh_node(&bvh_tree, scene.primitives, 0, scene.primitives_count, 0, 0);
    scene.bvh_tree = &bvh_tree;
    
    //~ Raytracer settings
    
    srand(NULL);
    
    u32 samples = 100;
    u32 depth = 50;
    
    RtRenderer rt_renderer;
    rt_renderer_init(&rt_renderer, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    
    RaytracerSettings rt_settings{};
    rt_settings.width   = TEXTURE_WIDTH;
    rt_settings.height  = TEXTURE_HEIGHT;
    rt_settings.samples = samples;
    rt_settings.depth   = depth;
    //rt_settings.image   = (r32*)rt_renderer.rt_backing[rt_renderer.rt_index];
    rt_settings.image   = (r32*)rt_renderer.rt_backing[0];
    rt_settings.scene   = &scene;
    rt_settings.camera  = &camera;
    
    //~ Setup the current app mode
    
    if (g_rt_mode == Mode::Offline)
    {
        rt_entry(&rt_settings);
        image_to_ppm(rt_settings.image, width, height);
        goto LBL_EXIT;
    }
    else if (g_rt_mode == Mode::OnlineBlocking)
    {
        rt_entry(&rt_settings);
    }
    else if (g_rt_mode == Mode::OnlineAsync)
    {
        rt_async_simple = CreateThread(NULL, 0, rt_online_async_proc, (void*)&rt_settings, 0, NULL);
    }
    else if (g_rt_mode == Mode::OnlineAsyncJob)
    {
        //rt_async_setup_jobs(128, 72, &rt_settings);
        rt_async_setup_jobs(192, 108, &rt_settings);
    }
    
    //~ BEGIN!
    
    r32 RefreshRate = 60.0f;
    r32 TargetSecondsPerFrame = 1 / RefreshRate;
    u64 frame_counter = 0;
    b8 last_frame_render = 1;
    Timer frame_timer;
    
    host_wnd_set_active(g_client);
    g_app_is_running = true;
    timer_begin(&frame_timer);
    
    while (g_app_is_running) 
    {
        // Handle all outstanding window messages.
        
        if (!host_wnd_msg_loop(g_client))
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
        
        r32 time_elapsed = timer_mili_seconds_elapsed(&frame_timer);
        
        //~ Render
        
        // Copy current version of image over
        if ((g_render_active > 0) && (frame_counter % 60) == 0)
        {
            if (g_rt_mode >= Mode::OnlineAsync)
            {
                rt_renderer_copy(&rt_renderer);
            }
        }
        
#if 1
        if (g_render_active == 0 && last_frame_render == 1)
        {
            // The frame is no longer rendering, but go ahead and perform
            // one last copy so that image is fully up-to-date
            if (g_rt_mode >= Mode::OnlineAsync)
            {
                rt_renderer_copy(&rt_renderer);
                LogInfo("Time to render: %lf", timer_seconds_elapsed(&rt_timer));
            }
            last_frame_render = 0;
        }
        else if (g_render_active == 1)
        {
            last_frame_render = 1;
        }
#endif
        
        // Set primary Render Target
        
        g_device_ctx->OMSetRenderTargets( 1, &g_gfx_primary_rt, NULL );
        
        D3D11_VIEWPORT Viewport;
        ZeroMemory(&Viewport, sizeof(D3D11_VIEWPORT));
        
        u32 Width, Height;
        PlatformGetWindowDims(&Width, &Height);
        Viewport.TopLeftX = 0;
        Viewport.TopLeftY = 0;
        Viewport.Width    = (r32)Width;
        Viewport.Height   = (r32)Height;
        g_device_ctx->RSSetViewports(1, &Viewport);
        
        g_device_ctx->ClearRenderTargetView(g_gfx_primary_rt, g_clear_color);
        g_device_ctx->ClearDepthStencilView(g_gfx_depth_stencil_view,  D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
        g_device_ctx->OMSetDepthStencilState(g_gfx_depth_stencil_state, 1);
        
        rt_renderer_draw(&rt_renderer);
        
        // TODO(Dustin): Determine if vsync is enabled
        g_swapchain->Present(0, 0);
        
        //~ Meet frame rate, if necessary
        
        r32 SecondsElapsedUpdate = timer_seconds_elapsed(&frame_timer);
        
        r32 SecondsElapsedPerFrame = SecondsElapsedUpdate;
        if (SecondsElapsedPerFrame < TargetSecondsPerFrame)
        {
            DWORD SleepMs = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecondsElapsedPerFrame));
            if (SleepMs > 0)
            {
                Sleep(SleepMs);
            }
            SecondsElapsedPerFrame = timer_seconds_elapsed(&frame_timer);
        }
        else
        {
            // LOG: Missed frame rate!
        }
        
        timer_begin(&frame_timer);
    }
    
    LBL_EXIT:;
    
    Win32ThreadPoolFree(&g_thread_pool);
    arrfree(job_data);
    rt_renderer_free(&rt_renderer);
    host_wnd_free(&g_client);
    SysMemoryFree();
    PlatformFree(app_backing_memory);
    PlatformLoggerFree();
    
    return (0);
}
