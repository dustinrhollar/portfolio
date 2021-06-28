
#define MAPLE_MEMORY_IMPLEMENTATION
#define MAPLE_STRING_IMPLEMENTATION
#define MAPLE_MATH_IMPLEMENTATION

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

#include "Platform/PlatformTypes.h"
#include "Core/Core.h"
#include "Platform/Platform.h"

#include "Util/Memory.h"
#include "Core/SysMemory.h"
#include "Util/String.h"
#include "Util/MapleMath.h"

#include "Platform/HostKey.h"
#include "Platform/HostWindow.h"
#include "Platform/PrettyBuffer.h"
#include "Platform/UniformBuffer.h"
#include "Platform/ControlBindings.h"

#include "Renderer/glad.c"
#include "GL/gl.h"

#include "Renderer/ShaderData.h"
#include "Renderer/ShaderProgram.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/Renderer.h"
#include "Renderer/Terrain/Perlin.h"
#include "Renderer/Terrain/TerrainGen.h"
#include "Renderer/Water/WaterGen.h"

#include "Core/SysMemory.c"
#include "Platform/PrettyBuffer.c"
#include "Platform/UniformBuffer.c"

#include "Renderer/ShaderProgram.c"
#include "Renderer/Framebuffer.c"
#include "Renderer/Renderer.c"
#include "Renderer/Terrain/Perlin.c"
#include "Renderer/Terrain/TerrainGen.c"
#include "Renderer/Water/WaterGen.c"


#include "Platform/Platform.c"
