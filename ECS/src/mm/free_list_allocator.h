#ifndef JENGINE_MM_FREE_LIST_ALLOCATOR_H
#define JENGINE_MM_FREE_LIST_ALLOCATOR_H

/*

FreeListAllocator maintains an implicit doubly 
linked list that tracks freed allocations. Freed
allocations are called "blocks" in the linked list.

When allocating memory, the free list is searched
for a block  of suitable size using first fit search. 
If a found block has a size more than 24bytes of extra 
space, then the found block is split and where the leftover 
memory is inserted back into the free list. If a block
 of suitable size is not found, then nullptr is returned.

Freed allocations are inserted into the freed list based
on their start address. If the previous and next block
are adjacent to the new block, then the blocks are merged
in order to reduve internal fragmentation.

A header for an allocation contains the following information:
1. Size of allocation: alignment/header + requested size
2. (Allocation Only) aligned length of the allocation, which is a mixture
 of alginment and the header.
3. (FreeList Only) Pointer to the next free block of memory
4. (FreeList Only) Pointer to the previous free block of memory

Alginment is only stored for used allocations and the next/previous
pointers are only stored for freed allocations.

Total size of the header is 24 bytes.

*/

#include "allocator.h"
#include <jackal_types.h>

namespace jengine { namespace mm {
        
        class FreeListAllocator : public Allocator 
        {
            public:
            
            FreeListAllocator(size_t size, void *start);
            ~FreeListAllocator();
            
            // Copy and Move not allowed for an allocator
            FreeListAllocator(FreeListAllocator & other) = delete;
            FreeListAllocator(FreeListAllocator && other) = delete;
            
            FreeListAllocator& operator=(FreeListAllocator & other) = delete;
            FreeListAllocator& operator=(FreeListAllocator && other) = delete;
            
            virtual void* Allocate(size_t size, size_t alignment) override;
            virtual void Free(void * ptr) override;
            
            private:
            
            // For allocated blocks, { Size, Alignment } is used
            // For free blocks, { Size, Next, Prev } is used
            struct Header {
                size_t Size;
                union {
                    u8 Alignment;
                    
                    struct 
                    {
                        Header *Next;
                        Header *Prev;
                    };
                };
            };
            
            inline Header* Mem2Header(void *mem) {return (Header*)((char*)mem-sizeof(Header));}
            inline void* Header2Mem(Header *header) {return (void*)((char*)header+sizeof(header));}
            
            inline bool CanMerge(Header *left, Header *right)
            {
                return ((char*)left + left->Size) == ((char*)right);
            }
            
            Header *FreeList;
        };
        
    } // mm
} // jengine

#endif // JENGINE_MM_FREE_LIST_ALLOCATOR_H
