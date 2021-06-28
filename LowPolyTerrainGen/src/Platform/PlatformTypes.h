
#ifndef _PLATFORM_TYPES_H
#define _PLATFORM_TYPES_H

#if defined(__linux__) || defined(__APPLE__) 

#define FORCE_INLINE inline __attribute__((always_inline))

#else
#error Platform not supported!
#endif

#endif //_PLATFORM_TYPES_H
