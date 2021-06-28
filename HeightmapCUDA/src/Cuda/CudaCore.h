#ifndef _CUDA_CORE_H
#define _CUDA_CORE_H

#include <stdint.h>
#include <float.h>
#include <assert.h>

#define GLOBAL          __global__
#define HOST            __host__
#define DEVICE          __device__
#define CONSTANT        __constant__
#define HOST_DEVICE     __host__ __device__
#define DEVICE_CONSTANT __constant__ __device__

#define FORCE_INLINE __forceinline

#define file_global   static
#define file_internal static
#define local_persist static

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

#define U8_MAX  UINT8_MAX
#define U16_MAX UINT16_MAX
#define U32_MAX UINT32_MAX
#define U64_MAX UINT64_MAX
#define I8_MAX  INT8_MAX
#define I16_MAX INT16_MAX
#define I32_MAX INT32_MAX
#define I64_MAX INT64_MAX
#define R32_MAX FLT_MAX

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

#define ARRAYCOUNT(x) (sizeof(x) / sizeof(x[0]))

// TODO(Dustin): Remove iostream somehow
#include <iostream>
#define CheckCudaErrors(val) CheckCuda((val), #val, __FILE__, __LINE__)
FORCE_INLINE void CheckCuda(cudaError_t result, char const *const func, const char *const file, int const line)
{
    if (result)
    {
        std::cerr << "CUDA error = " << static_cast<unsigned int>(result) << " at " << file << ":" << line << " '" << func << "' \n";
        cudaDeviceReset();
        exit(99);
    }
}

HOST_DEVICE FORCE_INLINE
r32 lerp(r32 v0, r32 v1, r32 t)
{
    return v0 + t * (v1 - v0);
}

HOST_DEVICE FORCE_INLINE
r32 inv_lerp(r32 v0, r32 v1, r32 v)
{
    r32 Result = 0.0f;
    Result = (v - v0) / (v1 - v0);
    Result = (Result < 0.0f) ? 0.0f : (Result > 1.0f) ? 1.0f : Result;
    return Result;
}

HOST_DEVICE FORCE_INLINE
r32 remap(r32 i0, r32 i1, r32 o0, r32 o1, r32 v)
{
    r32 t = inv_lerp(i0, i1, v);
    return lerp(o0, o1, t);
}

#endif //_CUDA_CORE_H
