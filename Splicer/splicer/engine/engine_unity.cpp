
// Unity build file for all engine code


//~ Memory Manager Headers
#include "mm/allocator.h"
#include "mm/stack_allocator.h"
#include "mm/linear_allocator.h"
#include "mm/free_list_allocator.h"
#include "mm/pool_allocator.h"
#include "mm/proxy_allocator.h"
#include "mm/vulkan_allocator.h"
#include "mm/mm.h"


//~ Utility Headers

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// jstring definitions
#define pstring_alloc(s)   palloc(s)
#define pstring_realloc(src, sz) prealloc((src), (sz))
#define pstring_free(p)    pfree(p)

#define tstring_alloc(s)   palloc(s)
#define tstring_realloc(s) prealloc(s)
#define tstring_free(p)    pfree(p)


#define JENGINE_UTILS_HASHTABLE_IMPLEMENTATION
#define USE_JENGINE_JSTRING_IMPLEMENTATION

#define JPI 3.1415926535

#include "utils/dynamic_array.h"
#include "utils/linked_list.h"
#include "utils/hashtable.h"
#include "utils/jstring.h"
#include "utils/jtuple.h"
#include "utils/vector_math.h"



//~ ECS Headers
#include "ecs/entity.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "ecs/ecs.h"


//~ Vulkan Headers

// Lines 15271-15305 have been turned off in order to compile
// Lines 15313-15321
// Lines 15329-15330
// Lines 15334-15335
// Lines 15344
// Lines 15349
// Lines 15360
#define VMA_IMPLEMENTATION

#ifdef VK_USE_PLATFORM_WIN32_KHR
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

HMODULE VulkanLibrary;

#else
#error "Platform not supported!"
#endif

#include <vulkan/vulkan.h>

#include "vk/vulkan_functions.h"
#include <vma/vk_mem_alloc.h>
#include "vk/splicer_vulkan.h"


//~ Memory Manager Source Files
#include "mm/stack_allocator.cpp"
#include "mm/linear_allocator.cpp"
#include "mm/free_list_allocator.cpp"
#include "mm/pool_allocator.cpp"
#include "mm/proxy_allocator.cpp"
#include "mm/vulkan_allocator.cpp"
#include "mm/mm.cpp"

//~ Utility Source Files
#include "utils/vector_math.cpp"

//~ ECS Source Files
#include "ecs/entity.cpp"
#include "ecs/component.cpp"
#include "ecs/system.cpp"
#include "ecs/ecs.cpp"


//~ Vulkan Source Files
#include "vk/vulkan_functions.cpp"
#include "vk/splicer_vulkan.cpp"
