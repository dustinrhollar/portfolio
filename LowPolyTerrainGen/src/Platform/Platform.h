#ifndef _PLATFORM_H
#define _PLATFORM_H

//------------------------------------------------------------------------------------
// Windowing API 

void PlatformGetWindowDims(u32 *width, u32 *height);

//------------------------------------------------------------------------------------
// FILE API 

typedef enum 
{
    // File Errors
    PlatformError_FileOpenFailure,
    PlatformError_FileCloseFailure,
    PlatformError_FileWriteFailure,
    PlatformError_FileReadFailure,
    PlatformError_FilePartialeWrite, // can occur if there is not enough disk space, or socket is blocked
    PlatformError_FilePartialeRead,
    PlatformError_DirectoryAlreadyExists,
    PlatformError_FileNotFound,
    PlatformError_PathNotFound,
    PlatformError_TooManyOpenFiles,
    PlatformError_AccessDenied,     
    PlatformError_InvalidHandle,
    PlatformError_MountNotFound,
    PlatformError_Success,
    PlatformError_Count,
    PlatformError_Unknown = PlatformError_Count,
} PlatformErrorType;

PlatformErrorType PlatformReadFileToBuffer(const char* file_path, u8** buffer, u32* size);
PlatformErrorType PlatformWriteBufferToFile(const char* file_path, u8* buffer, u64 size, bool append);

//------------------------------------------------------------------------------------
// LOGGING API

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define LogTrace(...) PlatformLog(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__) 
#define LogDebug(...) PlatformLog(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__) 
#define LogInfo(...)  PlatformLog(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__) 
#define LogWarn(...)  PlatformLog(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__) 
#define LogError(...) PlatformLog(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__) 
#define LogFatal(...) PlatformLog(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__) 

void PlatformLog(int level, const char *file, int line, const char *fmt, ...);

//------------------------------------------------------------------------------------
// BIT SHIFTING SHENANIGANS API

u32 PlatformCtz(u32 v); 
u32 PlatformClz(u32 v); 
u32 PlatformCtzl(u64 v); 
u32 PlatformClzl(u64 v); 

#endif //_PLATFORM_H
