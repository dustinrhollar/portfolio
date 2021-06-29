#include "free_list_allocator.h"
#include <assert.h>

namespace jengine { namespace mm {
        
        FreeListAllocator::FreeListAllocator(size_t size, void *start)
            : Allocator(size, start)
            , FreeList((Header*)StartAddr)
        {
            assert(size > sizeof(Header));
            
            FreeList->Size = size;
            FreeList->Next = nullptr;
            FreeList->Prev = nullptr;
        }
        
        FreeListAllocator::~FreeListAllocator()
        {
            FreeList = nullptr;
        }
        
        void *FreeListAllocator::Allocate(size_t size, size_t alignment) 
        {
            if (size + UsedMemory > Size)
                return nullptr;
            
            // Attempts to retrieve a value from the free list
            // using first fit search
            Header *iter = FreeList;
            while (iter != nullptr)
            {
                // aligned leng for the requested size
                u8 adj = AlignLength(iter, alignment, sizeof(Header)); 
                size_t total_size = iter->Size + adj;
                
                // found an address of suitable size
                if (size + adj <= iter->Size)
                    break;
                
                iter = iter->Next;
            }
            
            // if iter is null, then no free block of suitable size was not found
            if (!iter) return nullptr;
            
            // Remove iter from the free list
            if (iter == FreeList)
            { // iter is the front node
                FreeList = iter->Next;
                if (iter->Next)
                {
                    iter->Next->Prev = nullptr;
                }
            }
            else 
            {
                if (iter->Prev)
                {
                    iter->Prev->Next = iter->Next;
                }
                
                if (iter->Next)
                {
                    iter->Next->Prev = iter->Prev;
                }  
                
                iter->Next = nullptr;
                iter->Prev = nullptr;
            }
            
            // get the aligned address, length, and header
            u8 align_len = AlignLength(iter, alignment, sizeof(Header)); 
            void *adj_addr = (char*)iter + align_len;
            Header *header = Mem2Header(adj_addr);
            header->Size = iter->Size;
            header->Alignment = align_len;
            
            NumAllocations++;
            UsedMemory += header->Size;
            
            /*
            Split the Header if it can be split
        
            > header
                             > adj_addr
            |--------------------------------------|
            | align / Header | req_size | leftover |
            |--------------------------------------|
            | -- align_len - |
            | --------------- Size --------------- |
        
            sz_left = Size - (alignment + req_size)
            sz_new  = Size - sz_left;
        
            sp_blck = align_addr + req_size
        
            */
            size_t leftover = header->Size - (header->Alignment + size);
            if (leftover > sizeof(Header))
            { // node can be split
                void *split_node = (char*)adj_addr + size;
                
                // Create an allocation from the leftover and then
                // "Free" it to add it back to the list
                u8 tmp_align = AlignLength(split_node, 
                                           jengine::mm::DEFAULT_ALIGNMENT, 
                                           sizeof(Header)); // force 8byte alignment
                void *tmp_addr = (char*)split_node + tmp_align;
                Header *tmp_header = Mem2Header(tmp_addr);
                tmp_header->Alignment = jengine::mm::DEFAULT_ALIGNMENT;
                tmp_header->Size = leftover;
                
                // needs to be temparily adjusted to avoid errors in the free
                NumAllocations++;
                
                Free(tmp_addr);
                
                header->Size = header->Size - leftover;
                assert(header->Size >= size);
            }
            
            return adj_addr;
        }
        
        void FreeListAllocator::Free(void * ptr) 
        {
            Header *header = Mem2Header(ptr);
            size_t total_size = header->Size;
            
            // this will probably overwrite the above header since the
            // above header was address aligned and the free header is 
            // not guarenteed to be address aligned 
            Header *addr = (Header*)((char*)ptr - header->Alignment);
            addr->Size = total_size;
            
            // Insert into the free list based on address
            // sorted smallest to largest
            Header *iter = FreeList;
            if (iter) 
            { // just in case FreeList was null
                while (iter->Next != nullptr && iter->Next < addr)
                    iter = iter->Next;
            }
            
            if (!FreeList)
            { // First and only node was chosen for allocation - need to reset FreeList
                FreeList = addr;
                FreeList->Next = nullptr;
                FreeList->Prev = nullptr;
            }
            else if (iter == FreeList && FreeList > addr)
            { // stopped at the front of the list and iter needs to be inserted in front of FreeList
                addr->Next = FreeList;
                addr->Prev = nullptr;
                FreeList->Prev = addr;
                
                FreeList = addr;
            }
            else
            { // insert to the right of iter
                addr->Next = iter->Next;
                addr->Prev = iter;
                
                if (iter->Next)
                {
                    iter->Next->Prev = addr;
                }
                
                iter->Next = addr;
            }
            
            // If Possible, Merge blocks
            // if adjacent, blocks will attempt merge to the right and left
            // this means that at most, only one block can be merged to the 
            // left and to the right.
            
            // merge right
            iter = addr;
            if (iter->Next && CanMerge(iter, iter->Next))
            {
                iter->Size += iter->Next->Size;
                iter->Next = iter->Next->Next;
                
                if (iter->Next)
                {
                    iter->Next->Prev = iter;
                }
            }
            
            // merge left
            iter = iter->Prev;
            if (iter && CanMerge(iter, iter->Next))
            {
                iter->Size += iter->Next->Size;
                iter->Next = iter->Next->Next;
                
                if (iter->Next)
                {
                    iter->Next->Prev = iter;
                }
                
                iter = iter->Prev;
            }
            
            // Update memory tracking
            --NumAllocations;
            
            // Necessary for splits
            if (UsedMemory > 0)
                UsedMemory -= total_size;
        }
        
    } // mm
} // jengine