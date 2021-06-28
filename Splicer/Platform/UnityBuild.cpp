#define MAPLE_MEMORY_IMPLEMENTATION
#define MAPLE_STRING_IMPLEMENTATION
#define MAPLE_MATH_IMPLEMENTATION
#define MAPLE_OPENGL_IMPLEMENTATION
#define MAPLE_HASH_FUNCTION_IMPLEMENTATION
#define MAPLE_STR_POOL_IMPLEMENTATION
#define STB_DS_IMPLEMENTATION

#define STBDS_FREE(c,p)      SysMemoryRelease(p)
#define STBDS_REALLOC(c,p,s) SysMemoryRealloc(p,s)

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
#include "../Common/Core/Core.h"
#include "Platform/Platform.h"

#include "../Common/Util/Memory.h"
#include "../Common/Core/SysMemory.h"
#include "../Common/Util/String.h"
#include "../Common/Util/MapleMath.h"
#include "../Common/Util/stb_ds.h"
#include "../Common/Util/HashFunctions.h"
#include "../Common/Util/StrPool.h"

#include "Platform/Timer.h"
#include "Platform/HostKey.h"
#include "Platform/HostWindow.h"
#include "Platform/FileManager.h"
//#include "Platform/ControlBindings.h"

#include "../Common/Core/SysMemory.cpp"

// Platform Source

#include "Platform/Platform.cpp"
