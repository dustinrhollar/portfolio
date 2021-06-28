#include <errno.h>

void X11RequestMemory(void **result, u64 size) 
{
    *result = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0);

    if (*result == MAP_FAILED)
    {
        LogFatal("Failed to allocate memory with size %ld", size);
    }

    int ret = madvise(*result, size, MADV_WILLNEED);
    if (ret != 0)
    {
        LogError("Failed to advice mmap with adive MADV_WILLNEED");
    }
}

void X11ReleaseMemory(void **ptr, u64 size)
{
    int err = munmap(*ptr, size);
    if (err != 0)
    {
        const char *err_type = strerror(errno);
        LogFatal("Failed to unmap memory: %s!", err_type);
    }
    *ptr = 0;
}

u32 PlatformCtz(u32 v) 
{
    u32 result = 32;
    if (v > 0) result = __builtin_ctz(v);
    return result;
}

u32 PlatformClz(u32 v) 
{
    u32 result = 32;
    if (v > 0) result = 31 - __builtin_clz(v);
    return result;
}

u32 PlatformCtzl(u64 v) 
{
    u32 result = 64;
    if (v > 0) result = __builtin_ctzl(v);
    return result;
}

u32 PlatformClzl(u64 v)
{
    u32 result = 64;
    if (v > 0) result = 63 - __builtin_ctzl(v);
    return result;
}
