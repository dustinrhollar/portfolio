
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB               0x2093
#define WGL_CONTEXT_FLAGS_ARB                     0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB                 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#define WGL_DRAW_TO_WINDOW_ARB                    0x2001
#define WGL_ACCELERATION_ARB                      0x2003
#define WGL_SUPPORT_OPENGL_ARB                    0x2010
#define WGL_DOUBLE_BUFFER_ARB                     0x2011
#define WGL_PIXEL_TYPE_ARB                        0x2013

#define WGL_TYPE_RGBA_ARB                         0x202B
#define WGL_FULL_ACCELERATION_ARB                 0x2027

typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext,
                                                    const int *attribList);

typedef BOOL WINAPI wgl_choose_pixel_format_arb(HDC hdc,
                                                const int *piAttribIList,
                                                const FLOAT *pfAttribFList,
                                                UINT nMaxFormats,
                                                int *piFormats,
                                                UINT *nNumFormats);

typedef BOOL WINAPI wgl_swap_interval_ext(int interval);
typedef const char * WINAPI wgl_get_extensions_string_ext(void);

file_global wgl_create_context_attribs_arb *wglCreateContextAttribsARB;
file_global wgl_choose_pixel_format_arb *wglChoosePixelFormatARB;
file_global wgl_swap_interval_ext *wglSwapIntervalEXT;
file_global wgl_get_extensions_string_ext *wglGetExtensionsStringEXT;

file_global int Win32OpenGLAttribs[] =
{
    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
    WGL_CONTEXT_MINOR_VERSION_ARB, 3,
    WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#if !defined(NDEBUG)
    |WGL_CONTEXT_DEBUG_BIT_ARB
#endif
    ,
#if 0
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
#else
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#endif
    0,
};

// SOURCE: HANDMADE HERO
file_internal void Win32SetPixelFormat(Win32Window *window, HDC WindowDC)
{
    int SuggestedPixelFormatIndex = 0;
    GLuint ExtendedPick = 0;
    if(wglChoosePixelFormatARB)
    {
        int IntAttribList[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE, // 0
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB, // 1
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE, // 2
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE, // 3
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, // 4
            //WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE, // 5
            0,
        };
        
        wglChoosePixelFormatARB(WindowDC, IntAttribList, 0, 1,
                                &SuggestedPixelFormatIndex, &ExtendedPick);
    }
    
    if(!ExtendedPick)
    {
        // TODO(casey): Hey Raymond Chen - what's the deal here?
        // Is cColorBits ACTUALLY supposed to exclude the alpha bits, like MSDN says, or not?
        PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
        DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
        DesiredPixelFormat.nVersion = 1;
        DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
        DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
        DesiredPixelFormat.cColorBits = 32;
        DesiredPixelFormat.cAlphaBits = 8;
        DesiredPixelFormat.cDepthBits = 24;
        DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;
        
        SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);
    }
    
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    // NOTE(casey): Technically you do not need to call DescribePixelFormat here,
    // as SetPixelFormat doesn't actually need it to be filled out properly.
    DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex,
                        sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);
}

// SOURCE: HANDMADE HERO
file_internal void Win32LoadWGLExtensions(Win32Window *window)
{
    WNDCLASSA window_class = {};
    
    window_class.lpfnWndProc = DefWindowProcA;
    window_class.hInstance = GetModuleHandle(0);
    window_class.lpszClassName = "MapleWGLLoader";
    
    if(RegisterClassA(&window_class))
    {
        HWND Window = CreateWindowExA(
                                      0,
                                      window_class.lpszClassName,
                                      "Maple Merchant",
                                      0,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      0,
                                      0,
                                      window_class.hInstance,
                                      0);
        
        HDC WindowDC = GetDC(Window);
        Win32SetPixelFormat(window, WindowDC);
        HGLRC OpenGLRC = wglCreateContext(WindowDC);
        if(wglMakeCurrent(WindowDC, OpenGLRC))
        {
            wglChoosePixelFormatARB =
                (wgl_choose_pixel_format_arb *)wglGetProcAddress("wglChoosePixelFormatARB");
            wglCreateContextAttribsARB =
                (wgl_create_context_attribs_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
            wglGetExtensionsStringEXT = (wgl_get_extensions_string_ext *)wglGetProcAddress("wglGetExtensionsStringEXT");
            
#if 0
            if(wglGetExtensionsStringEXT)
            {
                char *Extensions = (char *)wglGetExtensionsStringEXT();
                char *At = Extensions;
                while(*At)
                {
                    while(IsWhitespace(*At)) {++At;}
                    char *End = At;
                    while(*End && !IsWhitespace(*End)) {++End;}
                    
                    umm Count = End - At;
                    
                    if(0) {}
                    else if(StringsAreEqual(Count, At, "WGL_EXT_framebuffer_sRGB")) {OpenGL->SupportsSRGBFramebuffer = true;}
                    else if(StringsAreEqual(Count, At, "WGL_ARB_framebuffer_sRGB")) {OpenGL->SupportsSRGBFramebuffer = true;}
                    
                    At = End;
                }
            }
#endif
            
            wglMakeCurrent(0, 0);
        }
        
        wglDeleteContext(OpenGLRC);
        ReleaseDC(Window, WindowDC);
        DestroyWindow(Window);
    }
}

file_internal void Win32InitOpenGL(Win32Window *window)
{
    HDC hdc = GetDC(window->handle);
    Win32SetPixelFormat(window, hdc);
    
    Win32LoadWGLExtensions(window);
    Win32SetPixelFormat(window, hdc);
    
    HGLRC ogl_rc = 0;
    if(wglCreateContextAttribsARB)
    {
        ogl_rc = wglCreateContextAttribsARB(hdc, 0, Win32OpenGLAttribs);
    }
    
    if(!ogl_rc)
    {
        ogl_rc = wglCreateContext(hdc);
    }
    
    window->ogl_ctx = hdc;
    window->render_ctx = ogl_rc;
    wglMakeCurrent(window->ogl_ctx, window->render_ctx);
    
    // Load glad library?
    if (!gladLoadGL())
    {
        PlatformFatalError("Hey....uh...glad failed to load opengl functions");
    }
    
}

file_internal void Win32ShutdownOpenGL(Win32Window *window)
{
    wglMakeCurrent(NULL, NULL);
    ReleaseDC(window->handle, window->ogl_ctx);
    wglDeleteContext(window->render_ctx);
}