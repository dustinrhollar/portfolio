#include "proxy_allocator.h"

namespace jengine { namespace mm {
        
        ProxyAllocator::ProxyAllocator(Allocator &allocator)
            : Allocator(allocator.GetSize(), allocator.GetStart())
            , InternalAllocator(allocator)
        {
        }
        
        ProxyAllocator::~ProxyAllocator()
        {
        }
        
        void * ProxyAllocator::Allocate(size_t size, size_t alignment)
        {
            NumAllocations++;
            size_t mem = InternalAllocator.GetUsedMemory();
            
            void *ptr = InternalAllocator.Allocate(size, alignment);
            UsedMemory += InternalAllocator.GetUsedMemory() - mem;
            
            return ptr;
        }
        
        void ProxyAllocator::Free(void * ptr)
        {
            NumAllocations--;
            size_t mem = InternalAllocator.GetUsedMemory();
            
            InternalAllocator.Free(ptr);
            UsedMemory -= mem - InternalAllocator.GetUsedMemory();
        }
        
    } // mm
} // jengine