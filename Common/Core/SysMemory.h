#ifndef _SYS_MEMORY_H
#define _SYS_MEMORY_H

#define MemAlloc(s)      SysMemoryAlloc((s))
#define MemFree(p)       (SysMemoryRelease((void*)(p)), p = NULL)
#define MemRealloc(p, s) MemReallocWrapperT((p), (s))

#ifdef __cplusplus

template<typename T> T* MemReallocWrapperT(T* ptr, u64 size)
{
    return (T*)SysMemoryRealloc((void*)ptr, size);
}

#else
#define MemReallocWrapperT(p, s) ((p) = SysMemoryRealloc((p), (s)))
#endif

void SysMemoryInit(void *ptr, u64 size);
void SysMemoryFree();

void* SysMemoryAlloc(u64 size);
void  SysMemoryRelease(void *ptr);
void* SysMemoryRealloc(void *ptr, u64 size);

#endif // _SYS_MEMORY_H
