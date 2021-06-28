
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

#pragma region Undefine Windows Macros
// Only undefine, if DXGIType.h has not been included yet
#ifndef __dxgitype_h__
#undef DXGI_STATUS_OCCLUDED
#undef DXGI_STATUS_CLIPPED
#undef DXGI_STATUS_NO_REDIRECTION
#undef DXGI_STATUS_NO_DESKTOP_ACCESS
#undef DXGI_STATUS_GRAPHICS_VIDPN_SOURCE_IN_USE
#undef DXGI_STATUS_MODE_CHANGED
#undef DXGI_STATUS_MODE_CHANGE_IN_PROGRESS
#undef DXGI_ERROR_INVALID_CALL
#undef DXGI_ERROR_NOT_FOUND
#undef DXGI_ERROR_MORE_DATA
#undef DXGI_ERROR_UNSUPPORTED
#undef DXGI_ERROR_DEVICE_REMOVED
#undef DXGI_ERROR_DEVICE_HUNG
#undef DXGI_ERROR_DEVICE_RESET
#undef DXGI_ERROR_WAS_STILL_DRAWING
#undef DXGI_ERROR_FRAME_STATISTICS_DISJOINT
#undef DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE
#undef DXGI_ERROR_DRIVER_INTERNAL_ERROR
#undef DXGI_ERROR_NONEXCLUSIVE
#undef DXGI_ERROR_NOT_CURRENTLY_AVAILABLE
#undef DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED
#undef DXGI_ERROR_REMOTE_OUTOFMEMORY
#undef D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS
#undef D3D11_ERROR_FILE_NOT_FOUND
#undef D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS
#undef D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD
#undef D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS
#undef D3D10_ERROR_FILE_NOT_FOUND
#endif
#pragma endregion

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <d3dcompiler.h>

#include "Win32/Win32Timer.c"
#include "Win32/Win32Logger.c"
#include "Win32/Win32File.c"
#include "Win32/Win32CoreUtils.c"
#include "Win32/Win32Window.c"
#include "Win32/Win32ThreadPool.cpp"
#include "Win32/Win32FileManager.cpp"
#include "Renderer/DX11/DX11Renderer.c"
#include "Renderer/DX11/DX11RenderTarget.cpp"
#include "Renderer/DX11/DX11Texture.cpp"
#include "Raytracer/RaytracerRenderer.cpp"
#include "Win32/Win32Main.c"

#else
#error Platform Not Supported!
#endif
