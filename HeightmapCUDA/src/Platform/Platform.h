#ifndef _PLATFORM_H
#define _PLATFORM_H

struct WindowRect
{
    u32 width;
    u32 height;
};

enum class EConsoleColor : u8
{
    Black = 0, White,
    DarkGrey, Grey,
    DarkRed, Red,
    DarkGreen, Green,
    DarkBlue, Blue,
    DarkCyan, Cyan,
    DarkPurple, Purple,
    DarkYellow, Yellow,
};

enum class PlatformError : u8
{
    // File Errors
    FileOpenFailure,
    FileCloseFailure,
    FileWriteFailure,
    FileReadFailure,
    FilePartialeWrite, // can occur if there is not enough disk space, or socket is blocked
    FilePartialeRead,
    
    DirectoryAlreadyExists,
    
    FileNotFound,
    PathNotFound,
    TooManyOpenFiles,
    AccessDenied,     
    InvalidHandle,
    
    MountNotFound,
    
    Success
};

void PlatformFatalError(const char*  message, ...);
void PlatformPrintMessage(const char* message, 
                          EConsoleColor text_color = EConsoleColor::Grey, 
                          EConsoleColor background_color = EConsoleColor::Black);
void PlatformPrintError(const char* message, 
                        EConsoleColor text_color = EConsoleColor::Red, 
                        EConsoleColor background_color = EConsoleColor::Black);

PlatformError PlatformReadFileToBuffer(const char* file_path, u8** buffer, u32* size);
PlatformError PlatformWriteBufferToFile(const char* file_path, u8* buffer, u64 size, bool append = false);
Str PlatformGetFullExecutablePath();
Str PlatformNormalizePath(const char* path);
// TODO(Matt): Replace these params with enums.
Str PlatformShowBasicFileDialog(int type = 0, int resource_type = -1);

bool PlatformShowAssertDialog(const char* message, const char* file, u32 line);
void PlatformShowErrorDialog(const char* message);

void PlatformGetWindowRect(WindowRect *rect);

void mprint(char *fmt, ...);
void mprinte(char *fmt, ...);

MAPLE_GUID PlatformGenerateGuid();
Str PlatformGuidToString(MAPLE_GUID guid);
MAPLE_GUID PlatformStringToGuid(const char* guid_str);

PlatformError PlatformCreateDirectory(const char *abs_path);

u32 PlatformClz(u32 Value);
u32 PlatformCtz(u32 Value);
u32 PlatformCtzl(u64 Value);
u32 PlatformClzl(u64 Value);

#endif //_PLATFORM_H


