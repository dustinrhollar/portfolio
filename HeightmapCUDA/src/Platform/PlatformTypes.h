#ifndef _PLATFORM_TYPES_H
#define _PLATFORM_TYPES_H

#if defined(__linux__) || defined(__APPLE__) 

typedef int file_id;
typedef GUID MAPLE_GUID;
#define INVALID_FILE_ID -1

#define FORCE_INLINE inline __attribute__((always_inline))

#elif defined(_WIN32)

#include <windows.h>

typedef HANDLE file_id;
typedef GUID MAPLE_GUID;
#define INVALID_FILE_ID INVALID_HANDLE_VALUE

#define FORCE_INLINE __forceinline

#else
#error Platform not supported!
#endif

#endif //_PLATFORM_TYPES_H
