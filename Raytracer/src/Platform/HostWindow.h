#ifndef _HOST_WINDOW_H
#define _HOST_WINDOW_H

struct HostWnd;

typedef void (*HostWndResizeCallback)(u32 width, u32 height);
typedef void (*HostWndKeyPressCallback)(MapleKey key);
typedef void (*HostWndKeyReleaseCallback)(MapleKey key);

typedef struct HostWndCallbacks {
    HostWndResizeCallback     resize;
    HostWndKeyPressCallback   press;
    HostWndKeyReleaseCallback release;
} HostWndCallbacks;

void host_wnd_init(struct HostWnd **wnd, u32 width, u32 height, const char *title);
void host_wnd_free(struct HostWnd **wnd);

void host_wnd_set_callbacks(struct HostWnd *window, HostWndCallbacks *callbacks);

void host_wnd_set_active(struct HostWnd *wnd);
bool host_wnd_msg_loop(struct HostWnd *wnd);

void host_wnd_set_callbacks(struct HostWnd *wnd, HostWndCallbacks *callbacks);

bool host_wnd_is_key_pressed(struct HostWnd *wnd, MapleKey key);
bool host_wnd_is_key_released(struct HostWnd *wnd, MapleKey key);

void host_wnd_get_dims(struct HostWnd *wnd, u32 *width, u32 *height);

#endif // _HOST_WINDOW_H
