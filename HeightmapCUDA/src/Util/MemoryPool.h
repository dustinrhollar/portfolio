#ifndef _MEMORY_POOL_H
#define _MEMORY_POOL_H

template<typename T>
struct MemoryPool
{
    u64    size;
    void  *start;
    void **pool;
    // TODO(Dustin): Wrap in debug define?
    // Memory Usage tracking
    u64    num_allocations;
    u64    used_memory;
    
    static void Init(MemoryPool *pool, u32 element_count);
    void Shutdown();
    
    T* Alloc();
    void Release(T** value);
};

template<typename T>
void MemoryPool<T>::Init(MemoryPool *pool, u32 element_count)
{
    pool->used_memory     = 0;
    pool->num_allocations = 0;
    pool->size           = memory_align(element_count * sizeof(T), 8);
    pool->start          = MemAlloc(pool->size);
    pool->pool           = (void**)pool->start;
    
    // Assign the next pointers in the free list
    void **iter = pool->pool;
    for (u32 i = 0; i < element_count-1; ++i)
    {
        *iter = (char*)iter + sizeof(T);
        iter = (void**)(*iter);
    }
    *iter = NULL;
}

template<typename T>
void MemoryPool<T>::Shutdown()
{
    size     = 0;
    MemFree(start);
    start    = NULL;
    pool     = NULL;
}

template<typename T>
T* MemoryPool<T>::Alloc()
{
    void *result = NULL;
    if (pool)
    {
        result = pool;
        pool = (void**)(*pool);
        used_memory += sizeof(T);
        num_allocations++;
    }
    return (T*)result;
}

template<typename T>
void MemoryPool<T>::Release(T** value)
{
    *((void**)(*value)) = pool;
    pool = (void**)(*value);
    used_memory -= sizeof(T);
    num_allocations--;
    *value = 0;
}

#endif //_MEMORY_POOL_H