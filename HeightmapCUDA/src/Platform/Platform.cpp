#include "Platform.h"
#include "Timer.h"

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <stdio.h>
#include <shobjidl_core.h>
#include <windowsx.h>

#include "Win32/Win32ImGui.h"
#include "Win32/Win32Window.h"

#include "Win32/Win32Timer.cpp"
#include "Win32/Win32Logger.cpp"
#include "Win32/Win32File.cpp"
#include "Win32/Win32OpenGL.cpp"
#include "Win32/Win32ImGui.cpp"
#include "Win32/Win32CoreUtils.cpp"
#include "Win32/Win32Main.cpp"

#else
#error Platform not supported!
#endif