#ifndef _HOST_WINDOW_H
#define _HOST_WINDOW_H

struct HostWnd;
typedef struct HostWnd* host_wnd_t;

typedef void (*HostWndResizeCallback)(u32 width, u32 height);
typedef void (*HostWndKeyPressCallback)(MapleKey key);
typedef void (*HostWndKeyReleaseCallback)(MapleKey key);

typedef struct HostWndCallbacks {
    HostWndResizeCallback     resize;
    HostWndKeyPressCallback   press;
    HostWndKeyReleaseCallback release;
} HostWndCallbacks;

void HostWndInit(struct HostWnd **wnd, u32 width, u32 height, const char *title);
void HostWndFree(struct HostWnd **wnd);
void HostWndSetCallbacks(struct HostWnd *wnd, HostWndCallbacks *callbacks);
void HostWndSetActive(struct HostWnd *wnd);
bool HostWndMsgLoop(struct HostWnd *wnd);
bool HostWndIsKeyPressed(struct HostWnd *wnd, MapleKey key);
bool HostWndIsKeyReleased(struct HostWnd *wnd, MapleKey key);
void HostWndGetDims(struct HostWnd *wnd, u32 *width, u32 *height);

#endif // _HOST_WINDOW_H
