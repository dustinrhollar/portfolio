#ifndef _SYS_MEMORY_H
#define _SYS_MEMORY_H

void SysMemoryInit(void *ptr, u64 size);
void SysMemoryFree();

void* MemAlloc(u64 size);
void MemFree(void *ptr);
void* MemRealloc(void *ptr, u64 size);

template<typename T> 
T* MemAlloc(u32 count = 1)
{
    return (T*)MemAlloc(sizeof(T) * count);
}

template<typename T>
T* MemRealloc(T *ptr, u32 count)
{
    return (T*)MemRealloc((void*)ptr, sizeof(T) * count);
}

template<typename T>
void MemFree(T* ptr)
{
    MemFree((void*)ptr);
}


#endif //_MEMORY_H
