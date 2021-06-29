#ifndef JENGINE_MM_PROXY_ALLOCATOR_H
#define JENGINE_MM_PROXY_ALLOCATOR_H

/*

A ProxyAllocator is a wrapper for an
Allocator object. Useful for if multiple
systems use the same allocator. A proxy
allocator can be made that stores a reference
to the main allocator. Each subsytem can then
have a ProxyAllocator, allowing them to indirectly
use the same allocator.

*/

#include "allocator.h"
#include <jackal_types.h>

namespace jengine { namespace mm {
        
        class ProxyAllocator : public Allocator 
        {
            public :
            
            ProxyAllocator(Allocator &allocator);
            ~ProxyAllocator();
            
            virtual void * Allocate(size_t size, size_t alignment) override;
            virtual void Free(void * ptr) override;
            
            private: 
            
            //Prevent copies because it might cause errors 
            ProxyAllocator& operator=(const ProxyAllocator&);
            
            Allocator &InternalAllocator;
        };
        
    } // mm
} // jengine


#endif // JENGINE_MM_PROXY_ALLOCATOR_H