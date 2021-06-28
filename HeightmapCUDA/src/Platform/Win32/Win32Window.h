#ifndef _WIN32_WINDOW_H
#define _WIN32_WINDOW_H

struct Win32Window 
{
    HWND  handle;
    RECT  rect;
    DWORD style;
    DWORD ex_style;
    bool  is_minimized;
    
    u32   width;
    u32   height;
    
    // OpenGl context
    HDC   ogl_ctx;
    HGLRC render_ctx;
    
    void (*Resize)();
};
extern Win32Window *g_client_window;

#endif //_WIN32_WINDOW_H
