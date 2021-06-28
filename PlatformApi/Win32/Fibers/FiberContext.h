#ifndef _PLATFROM_FIBERS_FIBER_CONTEXT_H
#define _PLATFROM_FIBERS_FIBER_CONTEXT_H

#include <immintrin.h>

struct FiberContext
{
    void* rip; // instruction pointer
    void* rsp; // stack ptr
    void* rbx;
    void* rbp; // frame/base ptr
    void* rdi; // dst index
    void* rsi; // src index
    
    void* r12; // R12-15  must be preserved
    void* r13;
    void* r14;
    void* r15;

    __m128i xmm6;
    __m128i xmm7;
    __m128i xmm8;
    __m128i xmm9;
    __m128i xmm10;
    __m128i xmm11;
    __m128i xmm12;
    __m128i xmm13;
    __m128i xmm14;
    __m128i xmm15;

    void *data;
};

extern "C" void save_fiber_ctx(FiberContext *ctx);
extern "C" void restore_fiber_ctx(FiberContext *ctx);
extern "C" void swap_fiber_ctx(FiberContext *save_ctx, FiberContext *restore_ctx);

#endif //_PLATFROM_FIBERS_FIBER_CONTEXT_H