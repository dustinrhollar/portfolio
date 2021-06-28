
#define WINDOW_NAME "Maple Heightmap"
#define WINDOW_CLASS_NAME "MAPLE_HEIGHTMAP_PROTOTYPE_WINDOW"

file_global bool g_app_is_running = false;
file_global bool g_needs_resized = false;
Win32Window *g_client_window = 0;

file_global PlayerMovement g_player_movement; // reset each frame

void Win32CreateWindow(Win32Window* window, i32 default_width, i32 default_height, TCHAR* title);
void Win32RegisterWindowClass();
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
file_internal void Win32Resize(u32 width, u32 height);
file_internal int Win32ImGuiWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

void Win32RegisterWindowClass() 
{
    WNDCLASSEX window_class = {};
    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = WindowProc;
    window_class.hInstance = GetModuleHandle(0);
    //window_class.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);
    window_class.hCursor = (HCURSOR)LoadImage(0, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    window_class.hIcon = (HICON)LoadImage(0, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    window_class.lpszClassName = WINDOW_CLASS_NAME;
    RegisterClassEx(&window_class);
}

void Win32CreateWindow(Win32Window *window, i32 default_width, i32 default_height, TCHAR *title) 
{
    // Get the current screen size, so we can center our window.
	int screen_width = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
    
    RECT rect = { 0, 0, default_width, default_height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	LONG width = rect.right - rect.left;
	LONG height = rect.bottom - rect.top;
	LONG window_x = (screen_width - width) / 2;
	LONG window_y = (screen_height - height) / 2;
	if (window_x < 0) window_x = CW_USEDEFAULT;
	if (window_y < 0) window_y = CW_USEDEFAULT;
	if (width < 0) width = CW_USEDEFAULT;
	if (height < 0) height = CW_USEDEFAULT;
    
    window->style = WS_OVERLAPPEDWINDOW;
    window->ex_style = WS_EX_OVERLAPPEDWINDOW;
    
    window->handle = CreateWindowEx(window->ex_style,
									WINDOW_CLASS_NAME,
									title,
									window->style,
									window_x,
									window_y,
									width,
									height,
									0,
									0,
									GetModuleHandle(0),
                                    window);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    Win32Window *window = NULL;
    if (message == WM_NCCREATE) 
    {
        LPCREATESTRUCT create_struct = (LPCREATESTRUCT)lparam;
		window = (Win32Window*)create_struct->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
    }
    else 
    {
        window = (Win32Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    if (window) 
    {
        if (Win32ImGuiWndProc(hwnd, message, wparam, lparam)) return true;
        
        switch (message) 
        {
            case WM_DESTROY: 
            {
                g_app_is_running = false;
                return 0;
            } break;
            
            case WM_PAINT:
			{
				RECT rect;
				GetClientRect(hwnd, &rect);
				ValidateRect(hwnd, &rect);
				return 0;
			}
            
            case WM_SIZE:
			{
				RECT rect = {};
				GetClientRect(window->handle, &rect);
				window->width = rect.right - rect.left;
				window->height = rect.bottom - rect.top;
				window->is_minimized = (wparam == SIZE_MINIMIZED);
				g_needs_resized = true;
				return 0;
			}
            
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            {
                bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
                //if (alt) GlobalPerFrameInput.KeyPress |= Key_Alt;
                
                switch (wparam)
                {
                    case 'W': g_player_movement.w = 1; break;
                    case 'S': g_player_movement.s = 1; break;
                    case 'A': g_player_movement.a = 1; break;
                    case 'D': g_player_movement.d = 1; break;
                    case 'Q': g_player_movement.q = 1; break;
                    
                    case VK_ESCAPE:
                    {
                        g_app_is_running = false;
                        return 0;
                    } break;
                }
                
            } break;
            
            case WM_SYSKEYUP:
            case WM_KEYUP:
            {
                switch (wparam)
                {
                    case 'W': g_player_movement.w = 0; break;
                    case 'S': g_player_movement.s = 0; break;
                    case 'A': g_player_movement.a = 0; break;
                    case 'D': g_player_movement.d = 0; break;
                    case 'Q': g_player_movement.q = 0; break;
                }
            } break;
            
            case WM_MOUSEMOVE:
            {
                g_player_movement.old_mouse_x = g_player_movement.mouse_x;
                g_player_movement.old_mouse_y = g_player_movement.mouse_y;
                g_player_movement.mouse_x = GET_X_LPARAM(lparam);
                g_player_movement.mouse_y = GET_Y_LPARAM(lparam);
                /* Win32 is pretty braindead about the x, y position that
                   it returns when the mouse is off the left or top edge
                   of the window (due to them being unsigned). therefore,
                   roll the Win32's 0..2^16 pointer co-ord range to the
                   more amenable (and useful) 0..+/-2^15. */
                if((i32)g_player_movement.mouse_x & 1 << 15) g_player_movement.mouse_x -= (1 << 16);
                if((i32)g_player_movement.mouse_y & 1 << 15) g_player_movement.mouse_y -= (1 << 16);
            } break;
            
            case WM_MOUSEWHEEL:
            {
                g_player_movement.mouse_scroll_delta += (float)GET_WHEEL_DELTA_WPARAM(wparam)/(float)WHEEL_DELTA;
                return 0;
            }
            case WM_MOUSEHWHEEL:
            {
                g_player_movement.mouse_hscroll_delta += (float)GET_WHEEL_DELTA_WPARAM(wparam)/(float)WHEEL_DELTA;
                return 0;
            }
        }
    }
    
    return DefWindowProc(hwnd, message, wparam, lparam);
}

file_internal void Win32SwapBuffers(HDC ctx)
{
    SwapBuffers(ctx);
}

file_internal void RunCudaKernel()
{
}

file_internal void InitOpenGlState(u32 width, u32 height,
                                   u32 *vao, u32 *pbo, u32 *textureid,
                                   Shader *shader)
{
    // Setup PBO for cuda to write into
    
    // set up vertex data parameter
    int num_texels = width*height;
    int num_values = num_texels * 4;
    int size_tex_data = sizeof(GLubyte) * num_values;
    
    // Generate a buffer ID called a PBO (Pixel Buffer Object)
    glGenBuffers(1,pbo);
    // Make this the current UNPACK buffer (OpenGL is state-based)
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, *pbo);
    // Allocate data for the buffer. 4-channel 8-bit image
    glBufferData(GL_PIXEL_UNPACK_BUFFER, size_tex_data, NULL, GL_DYNAMIC_COPY);
    cudaGLRegisterBufferObject( *pbo );
    
    // Setup VAO for quad rendering (tmp)
    
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);
    
    GLfloat vertices[] =
    { 
        -1.0f, -1.0f, 
        1.0f, -1.0f, 
        1.0f,  1.0f, 
        -1.0f,  1.0f, 
    };
    
    GLfloat texcoords[] = 
    { 
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };
    
    GLushort indices[] = { 0, 1, 3, 3, 1, 2 };
    
    // Non-Interleaved format
    // buffer 0: vertices
    // buffer 1: tex coords
    // buffer 2: element indices
    GLuint vertexBufferObjID[3];
    glGenBuffers(3, vertexBufferObjID);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)0, 2, GL_FLOAT, GL_FALSE, 0, 0); 
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjID[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
    glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBufferObjID[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Setup Texture that PBO is written to
    
    glGenTextures(1,textureid);
    glBindTexture(GL_TEXTURE_2D, *textureid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA,
                 GL_UNSIGNED_BYTE, NULL);
    
    // TODO(Dustin): Setup Shader Program
    const char *passthrough_vertex = "shaders/Passthrough.vert";
    const char *passthrough_frag = "shaders/Passthrough.frag";
    *shader = Shader::Init(passthrough_vertex, passthrough_frag);
    
    glBindVertexArray(0);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
    GlobalTimerDataSetup();
    
    //~ initialze memory for the application
    
    u64 app_memory_size = _16MB; // for now...
    void *app_memory = Win32RequestMemory(app_memory_size);
    SysMemoryInit(app_memory, app_memory_size);
    
    //~ Create the client window
    
    g_client_window = MemAlloc<Win32Window>();
    Win32RegisterWindowClass();
    Win32CreateWindow(g_client_window, 1600, 900, (TCHAR*)WINDOW_NAME);
    
    //~ initalize the renderer
    
    Win32InitOpenGL(g_client_window);
    
    // Choose device for CUDA
    i32 max_gflops = GpuGetMaxGflopsDeviceId();
    cudaGLSetGLDevice(max_gflops);
    
    u32 texture_width = 1600, texture_height = 900;
    u32 vao, pbo, display_img;
    Shader shader;
    
    InitOpenGlState(texture_width, texture_height, 
                    &vao, &pbo, &display_img,
                    &shader);
    
    // TODO(Dustin): Run Terrain CUDA on separate threads
    
    uchar4 *dptr = NULL;
    cudaGLMapBufferObject((void**)&dptr, pbo);
    //SimpleEntry(dptr, texture_width, texture_height);
    PerlinKernelEntry(dptr, texture_width, texture_height);
    
    // Upload the new texture to the GPU
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBindTexture(GL_TEXTURE_2D, display_img);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_width, texture_height, 
                    GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    //~ Initialize ImGui
    
    IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigWindowsMoveFromTitleBarOnly = true;
    ImGui::StyleColorsDark();
    
    if (!Win32ImGuiInit(g_client_window))
    {
        PlatformFatalError("Failed to initialize ImGui!\n");
    }
    
    OpenGlImGuiInit("#version 430");
    
    if (!OpenGlImGuiCreateFontsTexture())
    {
        PlatformFatalError("Failed to create ImGui Fonts Texture!\n");
        
    }
    
    // NOTE(Dustin): Figure out why the filename itself is not working...
    //io.Fonts->AddFontFromFileTTF("../../ext/imgui/misc/fonts/Roboto-Medium.ttf", 16.0f);
    
    //~ Show the window
    
    ShowWindow(g_client_window->handle, SW_SHOWNORMAL);
    g_app_is_running = true;
    
    //~ BEGIN!
    
    Timer frame_timer = {};
    frame_timer.Begin();
    
    while (g_app_is_running) 
    {
        // Every loop, handle all outstanding window messages.
		
        MSG msg = {};
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				g_app_is_running = false;
            }
		}
        
        if (g_needs_resized)
        {
            Win32Resize(g_client_window->width, g_client_window->height);
            g_needs_resized = false;
        }
        
        r32 time_elapsed = frame_timer.MiliSecondsElapsed();
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Render
        
        OpenGlImGuiNewFrame();
        Win32ImGuiNewFrame();
        ImGui::NewFrame();
        
        glBindVertexArray(vao);
        
        shader.Use();
        glDrawElements(GL_TRIANGLES, 6,  GL_UNSIGNED_SHORT, 0);
        
        // Draw the UI
        //EditorEntry(&asset_manager);
        
        ImGui::Render();
        OpenGlImGuiRenderDrawData(ImGui::GetDrawData());
        
        // Prep Next Frame
        Win32SwapBuffers(g_client_window->ogl_ctx);
    }
    
    //GameShutdown(NULL);
    
    Win32ImGuiShutdown();
    Win32ShutdownOpenGL(g_client_window);
    MemFree(g_client_window);
    g_client_window = 0;
    
    SysMemoryFree();
    Win32ReleaseMemory(app_memory, app_memory_size);
    
    return (0);
}

file_internal void Win32Resize(u32 width, u32 height)
{
    // TODO(Dustin): Handle different resolutions and enfore
    // window sizes here.
    
    // Set the viewport to be the app resolution 
    glViewport(0, 
               0, 
               g_client_window->width, 
               g_client_window->height);
}

void PlatformGetWindowRect(WindowRect *rect)
{
    assert(g_client_window->width != 0 && "Client Width is 0!");
    assert(g_client_window->height != 0 && "Client Height is 0!");
    
    rect->width = g_client_window->width;
    rect->height = g_client_window->height;
}

file_internal int Win32ImGuiWndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (ImGui::GetCurrentContext() == NULL) return 0;
    ImGuiIO& io = ImGui::GetIO();
    
    switch (message)
    {
        case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
        {
            int button = 0;
            if (message == WM_LBUTTONDOWN || message == WM_LBUTTONDBLCLK) {button = 0;}
            if (message == WM_RBUTTONDOWN || message == WM_RBUTTONDBLCLK) {button = 1;}
            if (message == WM_MBUTTONDOWN || message == WM_MBUTTONDBLCLK) {button = 2;}
            if (message == WM_XBUTTONDOWN || message == WM_XBUTTONDBLCLK) {button = (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) ? 3 : 4;}
            if (!ImGui::IsAnyMouseDown() && !GetCapture()) SetCapture(hwnd);
            io.MouseDown[button] = true;
            return 0;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        {
            int button = 0;
            if (message == WM_LBUTTONUP) {button = 0;}
            if (message == WM_RBUTTONUP) {button = 1;}
            if (message == WM_MBUTTONUP) {button = 2;}
            if (message == WM_XBUTTONUP) {button = (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) ? 3 : 4;}
            io.MouseDown[button] = false;
            if (!ImGui::IsAnyMouseDown() && GetCapture() == hwnd) ReleaseCapture();
            return 0;
        }
        case WM_MOUSEWHEEL:
        {
            io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA;
            return 0;
        }
        case WM_MOUSEHWHEEL:
        {
            io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA;
            return 0;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            if (wparam < 256) io.KeysDown[wparam] = 1;
            return 0;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            if (wparam < 256) io.KeysDown[wparam] = 0;
            return 0;
        }
        case WM_CHAR:
        {
            // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
            if (wparam > 0 && wparam < 0x10000)
                io.AddInputCharacterUTF16((unsigned short)wparam);
            return 0;
        }
        case WM_SETCURSOR:
        {
            if (LOWORD(lparam) == HTCLIENT && Win32ImGuiUpdateMouseCursor()) return 1;
            return 0;
        }
        case WM_DEVICECHANGE:
        {
            //if (wparam == DBT_DEVNODES_CHANGED) g_imgui_state.want_update_has_gamepad = true;
            return 0;
        }
        default: return 0;
    }
}
