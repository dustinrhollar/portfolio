
#include <GL/glx.h>
#include <GL/gl.h>

typedef struct HostWnd
{
    Display*          display;
    xcb_connection_t* connection;
    xcb_screen_t*     screen;
    xcb_window_t      window;
    xcb_atom_t        protocols;
    xcb_atom_t        delete_window;
    
    // OpenGL state
    GLXDrawable       drawable;
    GLXContext        context;


    // Window Settings
    u8 is_minimized:1;
    u8 is_fullscreen:1;
    u8 pad0:6;
    
    u32   width;
    u32   height;

    bool is_running;

    // Callbacks

    HostWndResizeCallback resize_callback;

    // Keyboard
    
    // Key input on linux is challenging...
    // /usr/share/X11/xkb/keycodes/evdev
    // ---- This file contains a set of general mapping from hardware keys
    // ---- to an identifier (ex: <ESC> = 9)
    // /usr/share/X11/xkb/symbols
    // ---- This directory maps the identifiers to named keys. For example,
    // ---- I am using the us keyboard layout.
    // These mapping are very useful in extending keyboard layouts but that means
    // there is not a real standard. 
    // In order to translate keybaord input, I feel like there are only really
    // two options:
    // 1. Hardcode a list of keyboard keys -> hardware keys
    // ---- No need to store extra state, but you can ONLY support locales with
    // ---- your mapping
    // 2. Store the loaded keys by opening a connection to the keyboard.
    // 
    // I have gone with option #2 for now. It is easier to work with and is more
    // extensible. My major concern is small memory allocations the xlib does behind
    // the scene. If this become a problem, I will need to move to #1.
    //
    // X11 keycodes can be found in two locations:
    // 1. https://code.woboq.org/qt5/include/X11/keysymdef.h.html#210
    // 2. /usr/include/X11/keysymdef.h
    //
    // NOTE(Dustin): Potential Bugs
    // ---- 1. On Key Press hold, there is a brief pause before more events are
    // ----    fired. Then key event alternate between Release and Press
    //
    xcb_key_symbols_t *key_syms;

    struct KeyboardState
    {
        u8 pressed[Key_Count];
        u8 released[Key_Count];
    } keyboard_state;

} HostWnd;


/*
    Attribs filter the list of FBConfigs returned by glXChooseFBConfig().
    Visual attribs further described in glXGetFBConfigAttrib(3)
*/
static int visual_attribs[] =
{
    GLX_X_RENDERABLE, True,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_STENCIL_SIZE, 8,
    GLX_DOUBLEBUFFER, True,
    GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, True,
    //GLX_SAMPLE_BUFFERS  , 1,
    //GLX_SAMPLES         , 4,
    None
};

typedef GLXContext (*glXCreateContextAttribsARBProc)
    (Display*, GLXFBConfig, GLXContext, Bool, const int*);


static void xcb_init_connection(HostWnd *wnd);
static xcb_intern_atom_cookie_t intern_atom_cookie(xcb_connection_t *c, Str *s);
static xcb_atom_t intern_atom(xcb_connection_t *c, xcb_intern_atom_cookie_t cookie);
static void xcb_handle_event(HostWnd *wnd, xcb_generic_event_t *ev);

#define XCB_UTF8_STRING_NAME "UTF8_STRING"
#define XCB_COOKIE_NAME      "WM_NAME"
#define XCB_PROTOCOL_NAME    "WM_PROTOCOLS"
#define XCB_ATOM_NAME        "WM_DELETE_WINDOW"

// Callback stubs
static void host_wnd_resize_stub(u32 width, u32 height) {}

void host_wnd_init(HostWnd **result, u32 width, u32 height, const char *title)
{
    Str utf8_name;
    Str atom_name;
    Str cookie_name;
    Str proto_name;

    int visualID = 0;
    Display *display;
    int default_screen;

    HostWnd *wnd = (HostWnd*)MemAlloc(sizeof(HostWnd));

    xcb_init_connection(wnd);
    wnd->window = xcb_generate_id(wnd->connection);

    /* Open Xlib Display */ 
    display = XOpenDisplay(0);
    if(!display)
    {
        LogFatal("Can't open display\n");
    }

    default_screen = DefaultScreen(display);

     /* Query framebuffer configurations that match visual_attribs */
    GLXFBConfig *fb_configs = 0;
    int num_fb_configs = 0;
    fb_configs = glXChooseFBConfig(display, default_screen, visual_attribs, &num_fb_configs);
    if(!fb_configs || num_fb_configs == 0)
    {
        LogFatal("glXGetFBConfigs failed\n");
    }

    /* Select first framebuffer config and query visualID */
    GLXFBConfig fb_config = fb_configs[0];
    glXGetFBConfigAttrib(display, fb_config, GLX_VISUAL_ID , &visualID);

    GLXContext context;

    /* Create OpenGL context */
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB =
        (glXCreateContextAttribsARBProc)
        glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");

    /* If we were on Winows, we would destroy the dummy context here. Again,
       this is unnecessary on Linux.*/

    if (!glXCreateContextAttribsARB) {
        printf("glXCreateContextAttribsARB() not found\n");
        exit(1);
    }

    /* Set desired minimum OpenGL version */
    static int context_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        None
    };
    /* Create modern OpenGL context */
    context = glXCreateContextAttribsARB(display, fb_config, NULL, true,
                                                context_attribs);
    if (!context) {
        printf("Failed to create OpenGL context. Exiting.\n");
        exit(1);
    }

#if 0
    context = glXCreateNewContext(display, fb_config, GLX_RGBA_TYPE, 0, True);
    if(!context)
    {
        LogFatal("glXCreateNewContext failed\n");
    }
#endif
    
    // create colormap
    xcb_colormap_t colormap = xcb_generate_id(wnd->connection);
    xcb_create_colormap(
            wnd->connection,
            XCB_COLORMAP_ALLOC_NONE,
            colormap,
            wnd->screen->root,
            visualID
            );

    u32 eventmask = 
        XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | 
        XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    u32 value_list[] = { eventmask, colormap, 0 };
    u32 value_mask = XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;

    
    xcb_create_window(wnd->connection, XCB_COPY_FROM_PARENT, wnd->window, wnd->screen->root,
                      0, 0, width, height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      visualID, value_mask, value_list);
    
    str_init(&utf8_name,   XCB_UTF8_STRING_NAME, strlen(XCB_UTF8_STRING_NAME));
    str_init(&cookie_name, XCB_COOKIE_NAME,      strlen(XCB_COOKIE_NAME));
    str_init(&proto_name,  XCB_PROTOCOL_NAME,    strlen(XCB_PROTOCOL_NAME));
    str_init(&atom_name,   XCB_ATOM_NAME,        strlen(XCB_ATOM_NAME));
    
    xcb_intern_atom_cookie_t utf8_string_cookie      = intern_atom_cookie(wnd->connection, &utf8_name);
    xcb_intern_atom_cookie_t wm_name_cookie          = intern_atom_cookie(wnd->connection, &cookie_name);
    xcb_intern_atom_cookie_t wm_protocols_cookie     = intern_atom_cookie(wnd->connection, &proto_name);
    xcb_intern_atom_cookie_t wm_delete_window_cookie = intern_atom_cookie(wnd->connection, &atom_name);
    
    // set title
    xcb_atom_t utf8_string = intern_atom(wnd->connection, utf8_string_cookie);
    xcb_atom_t wm_name = intern_atom(wnd->connection, wm_name_cookie);
    xcb_change_property(wnd->connection, XCB_PROP_MODE_REPLACE, wnd->window,
                        wm_name, utf8_string, 8, strlen(title), title);
    
    // advertise WM_DELETE_WINDOW
    wnd->protocols     = intern_atom(wnd->connection, wm_protocols_cookie);
    wnd->delete_window = intern_atom(wnd->connection, wm_delete_window_cookie);
    xcb_change_property(wnd->connection, XCB_PROP_MODE_REPLACE, wnd->window,
                        wnd->protocols, XCB_ATOM_ATOM, 32, 1, &wnd->delete_window);
    
    str_free(&utf8_name);
    str_free(&cookie_name);
    str_free(&proto_name);
    str_free(&atom_name);

    // NOTE: window must be mapped before glXMakeContextCurrent
    xcb_map_window(wnd->connection, wnd->window); 

    /* Create GLX Window */

    GLXWindow glxwindow = 
        glXCreateWindow(
            display,
            fb_config,
            wnd->window,
            0
        );

    if(!wnd->window)
    {
        LogFatal("glXDestroyContext failed\n");
    }

    wnd->display  = display;
    wnd->context  = context;
    wnd->drawable = glxwindow;

    /* make OpenGL context current */
    if(!glXMakeContextCurrent(wnd->display, wnd->drawable, wnd->drawable, wnd->context))
    {
        LogFatal("glXMakeContextCurrent failed\n");
    }

    // Set Window Callbacks to be stubs

    wnd->resize_callback = host_wnd_resize_stub;

    *result = wnd;
}

void host_wnd_free(HostWnd **wnd)
{
    glXDestroyWindow((*wnd)->display, (*wnd)->drawable);
    xcb_destroy_window((*wnd)->connection, (*wnd)->window);
    glXDestroyContext((*wnd)->display, (*wnd)->context);
    xcb_flush((*wnd)->connection);
    MemFree(*wnd);
    *wnd = 0;
}

bool host_wnd_msg_loop(HostWnd *wnd)
{
    xcb_generic_event_t *ev;
    while (wnd->is_running && (ev = xcb_poll_for_event(wnd->connection)))
    {
        xcb_handle_event(wnd, ev);
    }

    return wnd->is_running;
}

void host_wnd_set_active(HostWnd *wnd)
{
    xcb_map_window(wnd->connection, wnd->window);
    xcb_flush(wnd->connection);
    wnd->is_running = true;
}

bool host_wnd_is_key_pressed(HostWnd *wnd, MapleKey key)
{
    return (bool)wnd->keyboard_state.pressed[key];
}

bool host_wnd_is_key_released(HostWnd *wnd, MapleKey key)
{
    return (bool)wnd->keyboard_state.released[key];
}

static void xcb_init_connection(HostWnd *wnd)
{
    int scr;
    wnd->connection = xcb_connect(NULL, &scr);
    if (!wnd->connection || xcb_connection_has_error(wnd->connection)) {
        xcb_disconnect(wnd->connection);
        LogFatal("Failed to connect to the display server!");
    }
    
    const xcb_setup_t *setup = xcb_get_setup(wnd->connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    while (scr-- > 0) xcb_screen_next(&iter);
    
    wnd->screen = iter.data;
    
    // Get Key symbols
    wnd->key_syms = xcb_key_symbols_alloc(wnd->connection);
}

static xcb_intern_atom_cookie_t intern_atom_cookie(xcb_connection_t *c, Str *s)
{
    return xcb_intern_atom(c, false, s->len, str_to_string(s));
}

static xcb_atom_t intern_atom(xcb_connection_t *c, xcb_intern_atom_cookie_t cookie)
{
    xcb_atom_t atom = XCB_ATOM_NONE;
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(c, cookie, NULL);
    if (reply) {
        atom = reply->atom;
        free(reply);
    }
    return atom;
}

static MapleKey x11_key_to_maple_key(u32 x11_key)
{
    MapleKey result = Key_Unknown;

    if (x11_key >= XK_a && x11_key <= XK_z)
    {   
        result = Key_A + (x11_key - XK_a);
    }
    else if (x11_key >= XK_0 && x11_key <= XK_9)
    {
        result = Key_Zero + (x11_key - XK_0);
    }
    else if (x11_key >= XK_Left && x11_key <= XK_Down)
    {
        result = Key_Left + (x11_key - XK_Left);
    }
    else if (x11_key == XK_Shift_L || x11_key == XK_Shift_R)
    {
        result = Key_Shift;
    }
    else if (x11_key == XK_Control_L || x11_key == XK_Control_R)
    {
        result = Key_Ctrl;
    }
    else if (x11_key == XK_Alt_L || x11_key == XK_Alt_R)
    {
        result = Key_Alt;
    }
    else if (x11_key == XK_space)
    {
        result = Key_Space;
    }
    else if (x11_key == XK_Escape)
    {
        result = Key_Escape;
    }
    else if (x11_key == XK_Return)
    {
        result = Key_Return;
    }

    return result;
}

static void xcb_handle_event(HostWnd *wnd, xcb_generic_event_t *ev)
{
    // NOTE(Dustin): I would love to know why ~0x80 is used as a bitmask....
    switch (ev->response_type & ~0x80)
    {
        case XCB_CONFIGURE_NOTIFY: {
            xcb_configure_notify_event_t *notify = (xcb_configure_notify_event_t*)ev;
            if (notify->width != wnd->width || notify->height != wnd->height)
            {
                wnd->width = notify->width;
                wnd->height = notify->height;
                wnd->resize_callback(wnd->width, wnd->height);
            }
        } break;

        case XCB_CLIENT_MESSAGE: {
            xcb_client_message_event_t *msg = (xcb_client_message_event_t*)ev;
            if (msg->type == wnd->protocols && msg->data.data32[0] == wnd->delete_window) 
            {
                wnd->is_running = false;
            }
        } break;
    
        case XCB_KEY_PRESS: {
            xcb_key_press_event_t *kr = (xcb_key_press_event_t *)ev;
            
            xcb_keysym_t keysym = xcb_key_symbols_get_keysym(wnd->key_syms,
                                                             kr->detail, 0);

            MapleKey key = x11_key_to_maple_key(keysym);
            wnd->keyboard_state.pressed[key] = 1;
        } break;

        case XCB_KEY_RELEASE: {
            xcb_key_release_event_t *kr = (xcb_key_release_event_t *)ev;
            
            xcb_keysym_t keysym = xcb_key_symbols_get_keysym(wnd->key_syms,
                                                             kr->detail, 0);

            MapleKey key = x11_key_to_maple_key(keysym);
            wnd->keyboard_state.pressed[key]  = 0;
            wnd->keyboard_state.released[key] = 1;
        } break;
    }
}
