
#define WINDOW_CLASS_NAME "MAPLE_HEIGHTMAP_PROTOTYPE_WINDOW"
#include <dxgi.h>

// Heap allocated. Don't worry about size.
struct HostWnd 
{
    HWND             handle;
    RECT             rect;
    DWORD            style;
    DWORD            ex_style;
    
    // Window Settings
    u8               is_minimized:1;
    u8               is_fullscreen:1;
    u8               pad0:6;
    
    u32              width;
    u32              height;
    
    // NOTE(Dustin): 
    // Is the only member that can be modified on
    // the main + message thread. Since a write
    // is *very* rare (at least twice during 
    // app lifetime), I am not including sync
    // primitives because, at best, a memory
    // barrier will be introduced. It seems
    // unnecessary to insert a mem barrier for 
    // a rare write operation.
    //
    // Nonetheless, keep an eye on this for
    // possible a data race on shutdown.
    bool             is_running;
    
    // NOTE(Dustin):
    // Host Resize Callback. Same problem as above.
    // Resize callback might be set after MSG loop
    // takes over.
    HostWndCallbacks callbacks;
    
    // Key board mappings. Tracked via msg loop
    // Cache keyboard state to make it easer to track
    // outside of the message loop.
    struct KeyboardState
    {
        u8 pressed[Key_Count];  // 0: no, 1: yes
        u8 released[Key_Count]; // 0: no, 1: yes
    } keyboard;
};

file_internal void Win32RegisterWindowClass();
file_internal void Win32CreateWindow(HostWnd *window, i32 default_width, i32 default_height, TCHAR *title);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

void HostWndResizeStub(u32 width, u32 height) {}
void HostWndKeyPressStub(MapleKey key) {}
void HostWndKeyReleaseStub(MapleKey key) {}
void HostWndFullscreenStub() {}

ENGINE_INTERFACE void 
HostWndInit(struct HostWnd **window, u32 width, u32 height, const char *title)
{
    *window = (HostWnd*)SysAlloc(sizeof(struct HostWnd));
    (*window)->is_running = false;
    (*window)->is_fullscreen = 0;
    (*window)->is_minimized = 0;
    
    // Callback stubs
    ZeroMemory(&(*window)->callbacks, sizeof(HostWndCallbacks));
    (*window)->callbacks.resize  = HostWndResizeStub;
    (*window)->callbacks.press   = HostWndKeyPressStub;
    (*window)->callbacks.release = HostWndKeyReleaseStub;
    
    Win32RegisterWindowClass();
    Win32CreateWindow(*window, width, height, (TCHAR*)title);
}

ENGINE_INTERFACE void 
HostWndFree(struct HostWnd **window)
{
    SysFree(*window);
    *window = 0;
}

ENGINE_INTERFACE void*
HostWndGetRawHandle(struct HostWnd *wnd)
{
    return (void*)wnd->handle;
}

ENGINE_INTERFACE void 
HostWndSetCallbacks(HostWnd *window, HostWndCallbacks *callbacks)
{
    window->callbacks = *callbacks;
    
    if (!window->callbacks.resize)  window->callbacks.resize   = HostWndResizeStub;
    if (!window->callbacks.press)   window->callbacks.press    = HostWndKeyPressStub;
    if (!window->callbacks.release) window->callbacks.release  = HostWndKeyReleaseStub;
}

ENGINE_INTERFACE void 
HostWndSetActive(struct HostWnd *window)
{
    ShowWindow(window->handle, SW_SHOWNORMAL);
    window->is_running = true;
}

ENGINE_INTERFACE bool 
HostWndIsKeyPressed(HostWnd *window, MapleKey key)
{
    return (bool)window->keyboard.pressed[(u32)key];
}

ENGINE_INTERFACE bool 
HostWndIsKeyReleased(HostWnd *window, MapleKey key)
{
    return (bool)window->keyboard.released[(u32)key];
}

ENGINE_INTERFACE void 
HostWndGetDims(struct HostWnd *window, u32 *width, u32 *height)
{
    RECT rect;
    GetClientRect(window->handle, &rect);
    *width = rect.right - rect.left;
    *height = rect.bottom - rect.top;
}

ENGINE_INTERFACE bool 
HostWndMsgLoop(HostWnd *window)
{
    // Clear keyboard state
    memset(window->keyboard.pressed,  0, sizeof(u8) * (u32)Key_Count);
    memset(window->keyboard.released, 0, sizeof(u8) * (u32)Key_Count);
    
    // Process messages
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT)
        {
            window->is_running = false;
        }
    }
    return window->is_running;
}

void 
HostWndSetWindowZorderToTopMost(HostWnd* window, bool is_top)
{
    RECT rect;
    GetWindowRect(window->handle, &rect);
    
    SetWindowPos(window->handle,
                 (is_top) ? HWND_TOPMOST : HWND_NOTOPMOST,
                 rect.left,
                 rect.top,
                 rect.right - rect.left,
                 rect.bottom - rect.top,
                 SWP_FRAMECHANGED | SWP_NOACTIVATE);
}


// Convert a styled window into a fullscreen borderless window and back again.
void 
HostWndToggleFullscreen(HostWnd* window, IDXGISwapChain* pSwapChain)
{
    // WindowProc will set the fullscreen. If reverting from fullscreen,
    // this field will be FALSE
    if (!(bool)window->is_fullscreen)
    {
        // Restore the window's attributes and size.
        SetWindowLong(window->handle, GWL_STYLE, window->style);
        
        SetWindowPos(window->handle,
                     HWND_NOTOPMOST,
                     window->rect.left,
                     window->rect.top,
                     window->rect.right - window->rect.left,
                     window->rect.bottom - window->rect.top,
                     SWP_FRAMECHANGED | SWP_NOACTIVATE);
        
        ShowWindow(window->handle, SW_NORMAL);
    }
    else
    {
        // Save the old window rect so we can restore it when exiting fullscreen mode.
        GetWindowRect(window->handle, &window->rect);
        
        // Make the window borderless so that the client area can fill the screen.
        SetWindowLong(window->handle, GWL_STYLE, window->style & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));
        
        RECT fullscreenWindowRect;
        if (pSwapChain)
        {
            // Get the settings of the display on which the app's window is currently displayed
            IDXGIOutput *pOutput;
            ThrowIfFailed(pSwapChain->GetContainingOutput(&pOutput), "Failed to get containing output for swapchain");
            DXGI_OUTPUT_DESC Desc;
            ThrowIfFailed(pOutput->GetDesc(&Desc), "HostWndToggleFullscreen: Failed to get swapchain desc!");
            fullscreenWindowRect = Desc.DesktopCoordinates;
        }
        else
        {
            // Fallback to EnumDisplaySettings implementation
            LogError("Fallback to EnumDisplaySettings implementation");
            
            // Get the settings of the primary display
            DEVMODE devMode = {};
            devMode.dmSize = sizeof(DEVMODE);
            EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);
            
            fullscreenWindowRect = {
                devMode.dmPosition.x,
                devMode.dmPosition.y,
                devMode.dmPosition.x + static_cast<LONG>(devMode.dmPelsWidth),
                devMode.dmPosition.y + static_cast<LONG>(devMode.dmPelsHeight)
            };
        }
        
        SetWindowPos(window->handle,
                     HWND_TOPMOST,
                     fullscreenWindowRect.left,
                     fullscreenWindowRect.top,
                     fullscreenWindowRect.right,
                     fullscreenWindowRect.bottom,
                     SWP_FRAMECHANGED | SWP_NOACTIVATE);
        
        ShowWindow(window->handle, SW_MAXIMIZE);
    }
}

#if defined(D3D12_IMPLEMENTATION)

void 
HostWndD3D12MakeAssociation(host_wnd_t window, IDXGIFactory4 *factory, DeviceResources *graphics_device)
{
    // @ASSUMPTION:
    // With tearing support enabled we will handle ALT+Enter key presses in the
    // window message loop rather than let DXGI handle it by calling SetFullscreenState.
    factory->MakeWindowAssociation(window->handle, DXGI_MWA_NO_ALT_ENTER);
    window->graphics_device = graphics_device;
}

bool 
HostWndD3D12CreateSwapchain(host_wnd_t                              window, 
                            struct IDXGIFactory4                   *factory,
                            struct IUnknown                        *pDevice,
                            struct DXGI_SWAP_CHAIN_DESC1           *pDesc,
                            struct DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pFullscreenDesc,
                            IDXGIOutput                            *pRestrictToOutput,
                            IDXGISwapChain1                       **ppSwapChain)
{
    HRESULT res = factory->CreateSwapChainForHwnd(pDevice, 
                                                  window->handle, 
                                                  pDesc, 
                                                  pFullscreenDesc, 
                                                  pRestrictToOutput, 
                                                  ppSwapChain);
    if (SUCCEEDED(res)) return true;
    else                return false;
}
#endif

file_internal void 
Win32RegisterWindowClass() 
{
    WNDCLASSEX window_class;
    ZeroMemory(&window_class, sizeof(WNDCLASSEX));
    
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

file_internal void 
Win32CreateWindow(HostWnd *window, i32 default_width, i32 default_height, TCHAR *title) 
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

FORCE_INLINE MapleKey 
TranslateWin32Key(WPARAM key)
{
    MapleKey result = Key_Unknown;
    
    if (key >= 0x41 && key <= 0x5A)
    { // Key is between A and Z
        result = (MapleKey)((u32)Key_A + (key - 0x41));
    }
    else if (key >= 0x30 && key <= 0x39)
    { // Key is between 0 and 9
        result = (MapleKey)((u32)Key_Zero + (key - 0x30));
    }
    else if (key == VK_ESCAPE)
    {
        result = Key_Escape;
    }
    else if (key == VK_RETURN)
    {
        result = Key_Return;
    }
    else if (key == VK_SPACE)
    {
        result = Key_Space;
    }
    else if (key == VK_LEFT)
    {
        result = Key_Left;
    }
    else if (key == VK_RIGHT)
    {
        result = Key_Right;
    }
    else if (key == VK_UP)
    {
        result = Key_Up;
    }
    else if (key == VK_DOWN)
    {
        result = Key_Down;
    }
    
    return result;
};

LRESULT CALLBACK 
WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    HostWnd *window = NULL;
    if (message == WM_NCCREATE) 
    {
        LPCREATESTRUCT create_struct = (LPCREATESTRUCT)lparam;
        window = (HostWnd*)create_struct->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
    }
    else 
    {
        window = (HostWnd*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    if (window) 
    {
        //if (Win32WndProcHandler(window->handle, message, wparam, lparam))
        //return 0;
        
        switch (message) 
        {
            case WM_DESTROY: 
            {
                window->is_running = false;
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
                RECT rect;
                GetClientRect(window->handle, &rect);
                window->width = rect.right - rect.left;
                window->height = rect.bottom - rect.top;
                window->is_minimized = (wparam == SIZE_MINIMIZED);
                window->callbacks.resize(window->width, window->height);
                return 0;
            }
            
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            {
                bool alt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
                if (alt) window->keyboard.pressed[(u32)Key_Alt] = 1;
                
                switch (wparam)
                {
                    case VK_RETURN:
                    {
                        if (alt)
                        {
                            case VK_F11: 
                            {
                                // NOTE(Dustin): Workaround for not having a swapchain from graphics
                                HostWndToggleFullscreen(window, NULL);
                                window->callbacks.resize(window->width, window->height);
                            } break;
                        }
                    } break;
                }
                
                MapleKey key = TranslateWin32Key(wparam);
                window->keyboard.pressed[(u32)key] = 1;
                window->callbacks.press(key);
                
                return 0;
            } break;
            
            case WM_SYSKEYUP:
            case WM_KEYUP:
            {
                MapleKey key = TranslateWin32Key(wparam);
                window->keyboard.pressed[(u32)key]  = 0;
                window->keyboard.released[(u32)key] = 1;
                window->callbacks.release(key);
                return 0;
            } break;
            
            case WM_MOUSEMOVE:
            {
                // TODO(Dustin): 
                //g_player_movement.old_mouse_x = g_player_movement.mouse_x;
                //g_player_movement.old_mouse_y = g_player_movement.mouse_y;
                //g_player_movement.mouse_x = GET_X_LPARAM(lparam);
                //g_player_movement.mouse_y = GET_Y_LPARAM(lparam);
                /* Win32 is pretty braindead about the x, y position that
                   it returns when the mouse is off the left or top edge
                   of the window (due to them being unsigned). therefore,
                   roll the Win32's 0..2^16 pointer co-ord range to the
                   more amenable (and useful) 0..+/-2^15. */
                //if((i32)g_player_movement.mouse_x & 1 << 15) g_player_movement.mouse_x -= (1 << 16);
                //if((i32)g_player_movement.mouse_y & 1 << 15) g_player_movement.mouse_y -= (1 << 16);
            } break;
            
            case WM_MOUSEWHEEL:
            {
                // TODO(Dustin): 
                //g_player_movement.mouse_scroll_delta += (float)GET_WHEEL_DELTA_WPARAM(wparam)/(float)WHEEL_DELTA;
                return 0;
            }
            case WM_MOUSEHWHEEL:
            {
                // TODO(Dustin): 
                //g_player_movement.mouse_hscroll_delta += (float)GET_WHEEL_DELTA_WPARAM(wparam)/(float)WHEEL_DELTA;
                return 0;
            }
        }
    }
    
    return DefWindowProc(hwnd, message, wparam, lparam);
}
