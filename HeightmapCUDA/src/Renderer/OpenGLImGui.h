// dear imgui: Renderer Backend for modern OpenGL with shaders / programmatic pipeline

#ifndef _OPENGL_IMGUI
#define _OPENGL_IMGUI

#include "imgui/imgui.h"      // IMGUI_IMPL_API

// Backend API
IMGUI_IMPL_API bool OpenGlImGuiInit(const char* glsl_version = NULL);
IMGUI_IMPL_API void OpenGlImGuiShutdown();
IMGUI_IMPL_API void OpenGlImGuiNewFrame();
IMGUI_IMPL_API void OpenGlImGuiRenderDrawData(ImDrawData* draw_data);

// (Optional) Called by Init/NewFrame/Shutdown
IMGUI_IMPL_API bool OpenGlImGuiCreateFontsTexture();
IMGUI_IMPL_API void OpenGlImGuiDestroyFontsTexture();
IMGUI_IMPL_API bool OpenGlImGuiCreateDeviceObjects();
IMGUI_IMPL_API void OpenGlImGuiDestroyDeviceObjects();

#endif // _OPENGL_IMGUI