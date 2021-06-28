#define MAPLE_MEMORY_IMPLEMENTATION
#define MAPLE_STRING_IMPLEMENTATION
#define MAPLE_MATH_IMPLEMENTATION
#define MAPLE_OPENGL_IMPLEMENTATION
#define MAPLE_HASH_FUNCTION_IMPLEMENTATION
#define MAPLE_STR_POOL_IMPLEMENTATION
#define STB_DS_IMPLEMENTATION

#define STBDS_FREE(c,p)      SysMemoryRelease(p)
#define STBDS_REALLOC(c,p,s) SysMemoryRealloc(p, s)

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h> 
#include <stdarg.h>
#include <math.h>
#include <float.h>

// APPLICATION

#include "Platform/PlatformTypes.h"
#include "Core/Core.h"
#include "Platform/Platform.h"

#include "Util/Memory.h"
#include "Core/SysMemory.h"
#include "Util/String.h"
#include "Util/MapleMath.h"
#include "Util/stb_ds.h"
#include "Util/HashFunctions.h"
#include "Util/StrPool.h"

#include "Platform/Timer.h"
#include "Platform/HostKey.h"
#include "Platform/HostWindow.h"
#include "Platform/PrettyBuffer.h"
#include "Platform/UniformBuffer.h"
#include "Platform/FileManager.h"
//#include "Platform/ControlBindings.h"

#include "Core/SysMemory.c"
#include "Platform/PrettyBuffer.c"
#include "Platform/UniformBuffer.c"

#include "Renderer/ShaderCommon.h"
#include "Renderer/DX11/DX11Common.h"
#include "Renderer/DX11/DX11Texture.h"
#include "Renderer/DX11/DX11RenderTarget.h"
/* DX11Renderer.c defined in Platform.c */
#include "Renderer/DX11/DX11Renderer.h"

// RAYTRACER

#include "Raytracer/Material.h"
#include "Raytracer/Intersection.h"
#include "Raytracer/Primitive.h"
#include "Raytracer/Ray.h"
#include "Raytracer/Scene.h"
#include "Raytracer/Camera.h"
#include "Raytracer/Raytracer.h"
#include "Raytracer/RaytracerRenderer.h"

#include "Raytracer/Material.cpp"
#include "Raytracer/Primitive.cpp"
#include "Raytracer/Ray.cpp"
#include "Raytracer/Intersection.cpp"
#include "Raytracer/Scene.cpp"
#include "Raytracer/Camera.cpp"
#include "Raytracer/Raytracer.cpp"

// Platform Source

#include "Platform/Platform.c"
