
#if defined(__linux__) || defined(__APPLE__) 

#define __USE_GNU

#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <libgen.h>
#include <fcntl.h>
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>
#include <execinfo.h>
#include <unistd.h>
#include <signal.h>
#include <ucontext.h>

#include "X11/X11Logger.c"
#include "X11/X11Window.c"
#include "X11/X11CoreUtils.c"
#include "X11/X11File.c"
#include "X11/X11OpenGL.c"
#include "X11/X11Main.c"

#elif defined(_WIN32)

#define ThrowIfFailed(fn, msg) { HRESULT hr = (fn); if (FAILED(hr)) {LogFatal(msg);}}

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <stdio.h>
#include <shobjidl_core.h>
#include <windowsx.h>
#include <timeapi.h>

#include "Win32/Win32Timer.cpp"
#include "Win32/Win32Logger.cpp"
#include "Win32/Win32File.cpp"
#include "Win32/Win32CoreUtils.cpp"
#include "Win32/Win32Window.cpp"
#include "Win32/Win32ThreadPool.cpp"
#include "Win32/Win32FileManager.cpp"
#include "Win32/Win32Main.cpp"

#else
#error Platform Not Supported!
#endif
