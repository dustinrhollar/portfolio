#ifndef _SYS_MEMORY_H
#define _SYS_MEMORY_H

#define MemAlloc(s)      SysMemoryAlloc((s))
#define MemFree(p)       (SysMemoryRelease((p)), p = NULL)
#define MemRealloc(p, s) ((p) = SysMemoryRealloc((p), (s)))

void SysMemoryInit(void *ptr, u64 size);
void SysMemoryFree();

void* SysMemoryAlloc(u64 size);
void  SysMemoryRelease(void *ptr);
void* SysMemoryRealloc(void *ptr, u64 size);

#endif // _SYS_MEMORY_H
