#ifndef _HOST_WINDOW_H
#define _HOST_WINDOW_H

struct HostWnd;

typedef void (*HostWndResizeCallback)(u32 width, u32 height);

typedef struct HostWndCallbacks {
    HostWndResizeCallback resize;
} HostWndCallbacks;

void host_wnd_init(struct HostWnd **wnd, u32 width, u32 height, const char *title);
void host_wnd_free(struct HostWnd **wnd);

void host_wnd_set_active(struct HostWnd *wnd);
bool host_wnd_msg_loop(struct HostWnd *wnd);

bool host_wnd_is_key_pressed(struct HostWnd *wnd, MapleKey key);
bool host_wnd_is_key_released(struct HostWnd *wnd, MapleKey key);

#endif // _HOST_WINDOW_H
