#ifndef _UNITY_BUILD_H
#define _UNITY_BUILD_H

// Unity list of header files 

#include <stdint.h>
#include <assert.h>
#include <stdarg.h> 

// Some platform headers
#include "Platform/PlatformTypes.h"
#include "Core/EngineCore.h"
#include "Platform/Input.h"
#include "Platform/ByteBuffer.h"
#include "Platform/Timer.h"

// Util Headers

#include "Core/SysMemory.h"

#define HMM_PREFIX
#define HANDMADE_MATH_IMPLEMENTATION
#include "Util/HandmadeMath.h"
#undef HANDMADE_MATH_IMPLEMENTATION

#define MAPLE_FIXED_POINT_IMPLEMENTATION
#include "Util/FixedPoint.h"
#undef MAPLE_FIXED_POINT_IMPLEMENTATION

#define MAPLE_HASH_FUNCTION_IMPLEMENTATION
#include "Util/HashFunctions.h"
#undef MAPLE_HASH_FUNCTION_IMPLEMENTATION

// TODO(Dustin): Tear out UString in favor of my own.
// UString utilizes desctructors, which do not play well with
// my allocator.

#define USTRING_MALLOC(x)     MemAlloc((x))
#define USTRING_REALLOC(p, x) MemRealloc((p), (x))
#define USTRING_FREE(p)       MemFree((p))
#define USTRING_IMPLEMENTATION
#include "Util/UString.h"
#undef USTRING_IMPLEMENTATION
//#undef USTRING_FREE
//#undef USTRING_REALLOC
//#undef USTRING_MALLOC

#include "Platform/Platform.h"

#define MAPLE_MEMORY_IMPLEMENTATION
#include "Util/Memory.h"
#undef MAPLE_MEMORY_IMPLEMENTATION

#include "Util/MemoryPool.h"

#define STBDS_REALLOC(c,p,x) MemRealloc((void*)(p), (x))
#define STBDS_FREE(c,p)      MemFree((p))
#define STB_DS_IMPLEMENTATION
#include "Util/stb_ds.h"
#undef STB_DS_IMPLEMENTATION
//#undef STBDS_FREE
//#undef STBDS_REALLOC

#define MAPLE_STR_POOL_IMPLEMENTATION
#include "Util/StrPool.h"
#undef MAPLE_STR_POOL_IMPLEMENTATION

// Core
#include "Core/FrameParams.h"

// Renderer
#include "Renderer/glad/glad.h"
#include "Renderer/OpenGLImGui.h"
#include "Renderer/Shader.h"

// Editor

#include "imgui/imgui.h"
//#include "Editor/Editor.h"

#include "Core/EngineCoreSettings.h"

// CUDA

#include "Cuda/CudaUtils.h"
#include "Cuda/CudaInterop.cuh"

#endif //_UNITY_BUILD_H
