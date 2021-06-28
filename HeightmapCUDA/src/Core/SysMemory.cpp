
file_global memory_t ApplicationMemory = 0;

//~ Exposed API

void SysMemoryInit(void *ptr, u64 size)
{
    memory_init(&ApplicationMemory, size, ptr);
}

void SysMemoryFree()
{
    memory_free(&ApplicationMemory);
}

void* MemAlloc(u64 size)
{
    return memory_alloc(ApplicationMemory, size);
}

void MemFree(void *ptr)
{
    memory_release(ApplicationMemory, ptr);
}

void* MemRealloc(void *ptr, u64 size)
{
    return memory_realloc(ApplicationMemory, ptr, size);
}
