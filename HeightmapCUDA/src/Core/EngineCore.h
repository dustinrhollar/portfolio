#ifndef _ENGINE_CORE_H
#define _ENGINE_CORE_H

// Build type and their defines
// TODO(Dustin): Move to build script?
#define EDITOR_BUILD

#define MAPLE_ASSET_META_EXTENSION    ".ass"
#define MAPLE_ASSET_PROJECT_EXTENSION ".maple"
#define MAPLE_ASSET_SCENE_EXTENSION   ".scene"
#define MAPLE_ASSET_GLSL_EXTENSION    ".glsl" // i no likey
#define MAPLE_ASSET_SHADER_EXTENSION  ".shader"
#define MAPLE_ASSET_TERRAIN_EXTENSION ".terrain"

#define MAPLE_STARTUP_FILE                 "maple_startup.json"
#define MAPLE_STARTUP_PROJECT_DEFAILT_NAME "Maple Default Project Name"

#define file_global   static
#define file_internal static
#define local_persist static

#define U8_MAX  UINT8_MAX
#define U16_MAX UINT16_MAX
#define U32_MAX UINT32_MAX
#define U64_MAX UINT64_MAX
#define I8_MAX  INT8_MAX
#define I16_MAX INT16_MAX
#define I32_MAX INT32_MAX
#define I64_MAX INT64_MAX
#define OUTPUT_BUFFER_SIZE 2048

#define ARRAYCOUNT(x) (sizeof(x) / sizeof(x[0]))

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  r32;
typedef double r64;

typedef uintptr_t uptr;
typedef intptr_t  iptr;

typedef union u128
{
    struct { i64 upper, lower; };
    i64 Bits[2];
} u128;

// Defines for calculating size
#define _KB(x) (x * 1024)
#define _MB(x) (_KB(x) * 1024)
#define _GB(x) (_MB(x) * 1024)

#define _64KB  _KB(64)
#define _1MB   _MB(1)
#define _2MB   _MB(2)
#define _4MB   _MB(4)
#define _8MB   _MB(8)
#define _16MB  _MB(16)
#define _32MB  _MB(32)
#define _64MB  _MB(64)
#define _128MB _MB(128)
#define _256MB _MB(256)
#define _1GB   _GB(1)

#define memory_align(val, alignment) (((alignment) + (val) - 1) & ~((alignment) - 1))

#define BIT(x) 1<<(x)

#define BIT_TOGGLE(n, b, v) ((n) = ((n) & ~(1ULL << (b))) | ((v) << (b)))
#define BIT32_TOGGLE_0(n ,b) BIT_TOGGLE(n, b, 0)
#define BIT32_TOGGLE_1(n, b) BIT_TOGGLE(n, b, 1)
#define BIT64_TOGGLE_0(n ,b) BIT_TOGGLE(n, b, 0)
#define BIT64_TOGGLE_1(n, b) BIT_TOGGLE(n, b, 1ULL)

#define BITMASK_CLEAR(x,m) ((x) &=(~(m)))

#define SINGLE_ARG(...) __VA_ARGS__
#if defined(EDITOR_BUILD)

#define EDITOR_DEP_FIELD(type, name) type name
#define EDITOR_DEP_FN(fn) fn
#define EDITOR_DEP_FN_CALL(fn) (fn)

#else

#define EDITOR_DEP_FIELD(type, name)
#define EDITOR_DEP_FN(fn)
#define EDITOR_DEP_FN_CALL(fn)

#endif

#ifndef NDEBUG
#define Assert(x)                                                                  \
{                                                                                  \
    if (!(x))                                                                      \
    {                                                                              \
        if (PlatformShowAssertDialog(#x, __FILE__, (u32)__LINE__)) DebugBreak();   \
    }                                                                              \
}
#else
#define Assert(x)
#endif // NDEBUG

#ifndef NDEBUG
#define AssertCustom(x, message)                                                            \
{                                                                                       \
    if (!(x))                                                                           \
    {                                                                                   \
        if (PlatformShowAssertDialog(message, __FILE__, (u32)__LINE__)) DebugBreak(); \
    }                                                                                   \
}
#else
#define AssertCustom(x, message)
#endif // NDEBUG


// Helper for bitfield enum types. Takes an enum name (x) and underlying type
// (y), and overloads the bitwise operators.
#define BITFIELD_ENUM(x, y) enum struct x : y;\
x operator|(x a, x b) { return static_cast<x>(static_cast<y>(a) | static_cast<y>(b)); }\
x operator&(x a, x b) { return static_cast<x>(static_cast<y>(a) & static_cast<y>(b)); }\
x operator^(x a, x b) { return static_cast<x>(static_cast<y>(a) ^ static_cast<y>(b)); }\
x operator~(x a) { return static_cast<x>(~static_cast<y>(a)); }\
x##& operator|=(x##& a,x b) { return a = a | b; }\
x##& operator&=(x##& a,x b) { return a = a & b; }\
x##& operator^=(x##& a,x b) { return a = a ^ b; }\
bool operator==(y a, x b) {return static_cast<x>(a) == b;}\
bool operator==(x a, y b) {return static_cast<x>(b) == a;}\
bool operator!=(y a, x b) {return static_cast<x>(a) != b;}\
bool operator!=(x a, y b) {return static_cast<x>(b) != a;}\
enum struct x : y

FORCE_INLINE
r32 lerp(r32 v0, r32 v1, r32 t)
{
    return v0 + t * (v1 - v0);
}

FORCE_INLINE
r32 inv_lerp(r32 v0, r32 v1, r32 v)
{
    r32 Result = 0.0f;
    
    Result = (v - v0) / (v1 - v0);
    Result = (Result < 0.0f) ? 0.0f : (Result > 1.0f) ? 1.0f : Result;
    
    return Result;
}

FORCE_INLINE
r32 remap(r32 i0, r32 i1, r32 o0, r32 o1, r32 v)
{
    r32 t = inv_lerp(i0, i1, v);
    return lerp(o0, o1, t);
}

// source: https://hbfs.wordpress.com/2008/08/05/branchless-equivalents-of-simple-functions/
// short for sign extend :p
// forces the compile to use cbw family of instructions (ideally)
FORCE_INLINE
int sex(int x)
{
    union
    {
        // assumed long is twice as large as int
        long long w;
        struct { int lo, hi; } _p;
    }  z = {};
    z.w = x;
    return z._p.hi;
}

FORCE_INLINE
i32 fast_sign(i32 v)
{
    return (((u64)-v >> 31) - ((u64)v >> 31));
}

FORCE_INLINE
i32 fast_sign(i64 v)
{
    return (((u64)-v >> 63) - ((u64)v >> 63));
}

FORCE_INLINE
int fast_abs(int x)
{
    int result;
    result = (x ^ sex(x)) - sex(x);
    return result;
}

FORCE_INLINE
int fast_max(int min, int max)
{
    int result;
    result = min + ((max - min) & ~sex(max - min));
    return result; 
}

FORCE_INLINE
int fast_min(int min, int max)
{
    int result;
    result = max + ((min - max) & sex(min - max));
    return result;
}

FORCE_INLINE
int fast_clamp(int min, int max, int val)
{
    return fast_min(fast_max(val, min), max);
}


#endif //_ENGINE_CORE_H
