
#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE 512
#endif

struct Win32InternalAlloc
{
    char *start;
    char *brkp;
    
    static const u32 InternalSize = _1MB;
};

struct Win32StandardStream
{
    HANDLE handle; // Stream handle (STD_OUTPUT_HANDLE or STD_ERROR_HANDLE).
    bool is_redirected; // True if redirected to file.
    bool is_wide; // True if appending to a UTF-16 file.
    bool is_little_endian; // True if file is UTF-16 little endian.
};

// An internal allocation scheme that allocates a small amount of memory. 
// Memory is treated as a linear allocator.
// this functions is primarily used by print/formatting functions that need temporary,
// dynamic memory. When the heap is filled, the allocator is reset.
file_internal void* PlatformLocalAlloc(u32 size)
{
    static Win32InternalAlloc internal_allocator = {0};
    
    if (!internal_allocator.start)
    {
        internal_allocator.start = (char*)VirtualAlloc(NULL,                   
                                                       Win32InternalAlloc::InternalSize,
                                                       MEM_COMMIT|MEM_RESERVE,
                                                       PAGE_READWRITE);
        internal_allocator.brkp = internal_allocator.start;
    }
    
    u64 current_size = internal_allocator.brkp - internal_allocator.start;
    if (current_size + size > Win32InternalAlloc::InternalSize)
    {
        internal_allocator.brkp = internal_allocator.start;
    }
    
    void *result = internal_allocator.brkp;
    internal_allocator.brkp += size;
    
    return result;
}

// Sets up a standard stream (stdout or stderr).
static Win32StandardStream Win32GetStandardStream(u32 stream_type)
{
    Win32StandardStream result = {};
    
    // If we don't have our own stream and can't find a parent console,
    // allocate a new console.
    result.handle = GetStdHandle(stream_type);
    if (!result.handle || result.handle == INVALID_HANDLE_VALUE)
    {
        if (!AttachConsole(ATTACH_PARENT_PROCESS)) AllocConsole();
        result.handle = GetStdHandle(stream_type);
    }
    
    // Check if the stream is redirected to a file. If it is, check if
    // the file already exists. If so, parse the encoding.
    if (result.handle != INVALID_HANDLE_VALUE)
    {
        DWORD type = GetFileType(result.handle) & (~FILE_TYPE_REMOTE);
        DWORD dummy;
        result.is_redirected = (type == FILE_TYPE_CHAR) ? !GetConsoleMode(result.handle, &dummy) : true;
        if (type == FILE_TYPE_DISK)
        {
            LARGE_INTEGER file_size;
            GetFileSizeEx(result.handle, &file_size);
            if (file_size.QuadPart > 1)
            {
                u16 bom = 0;
                SetFilePointerEx(result.handle, {}, 0, FILE_BEGIN);
                ReadFile(result.handle, &bom, 2, &dummy, 0);
                SetFilePointerEx(result.handle, {}, 0, FILE_END);
                result.is_wide = (bom == (u16)0xfeff || bom == (u16)0xfffe);
                result.is_little_endian = (bom == (u16)0xfffe);
            }
        }
    }
    return result;
}

// Translates foreground/background color into a WORD text attribute.
static WORD Win32TranslateConsoleColors(EConsoleColor text_color, EConsoleColor background_color)
{
    WORD result = 0;
    switch (text_color)
    {
        case EConsoleColor::White:
        result |=  FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        break;
        case EConsoleColor::DarkGrey:
        result |= FOREGROUND_INTENSITY;
        break;
        case EConsoleColor::Grey:
        result |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        break;
        case EConsoleColor::DarkRed:
        result |= FOREGROUND_RED;
        break;
        case EConsoleColor::Red:
        result |= FOREGROUND_RED | FOREGROUND_INTENSITY;
        break;
        case EConsoleColor::DarkGreen:
        result |= FOREGROUND_GREEN;
        break;
        case EConsoleColor::Green:
        result |= FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        break;
        case EConsoleColor::DarkBlue:
        result |= FOREGROUND_BLUE;
        break;
        case EConsoleColor::Blue:
        result |= FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        break;
        case EConsoleColor::DarkCyan:
        result |= FOREGROUND_GREEN | FOREGROUND_BLUE;
        break;
        case EConsoleColor::Cyan:
        result |= FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        break;
        case EConsoleColor::DarkPurple:
        result |= FOREGROUND_RED | FOREGROUND_BLUE;
        break;
        case EConsoleColor::Purple:
        result |= FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        break;
        case EConsoleColor::DarkYellow:
        result |= FOREGROUND_RED | FOREGROUND_GREEN;
        break;
        case EConsoleColor::Yellow:
        result |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        break;
        default:
        break;
    }
    
    switch (background_color)
    {
        case EConsoleColor::White:
        result |=  FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        break;
        case EConsoleColor::DarkGrey:
        result |=  FOREGROUND_INTENSITY;
        break;
        case EConsoleColor::Grey:
        result |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        break;
        case EConsoleColor::DarkRed:
        result |= FOREGROUND_RED;
        break;
        case EConsoleColor::Red:
        result |= FOREGROUND_RED | FOREGROUND_INTENSITY;
        break;
        case EConsoleColor::DarkGreen:
        result |= FOREGROUND_GREEN;
        break;
        case EConsoleColor::Green:
        result |= FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        break;
        case EConsoleColor::DarkBlue:
        result |= FOREGROUND_BLUE;
        break;
        case EConsoleColor::Blue:
        result |= FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        break;
        case EConsoleColor::DarkCyan:
        result |= FOREGROUND_GREEN | FOREGROUND_BLUE;
        break;
        case EConsoleColor::Cyan:
        result |= FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        break;
        case EConsoleColor::DarkPurple:
        result |= FOREGROUND_RED | FOREGROUND_BLUE;
        break;
        case EConsoleColor::Purple:
        result |= FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        break;
        case EConsoleColor::DarkYellow:
        result |= FOREGROUND_RED | FOREGROUND_GREEN;
        break;
        case EConsoleColor::Yellow:
        result |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        break;
        default:
        break;
    }
    
    return result;
}

// Prints a message to a platform stream. If the stream is a console, uses
// supplied colors.
static void Win32PrintToStream(const char* message, Win32StandardStream stream, EConsoleColor text_color, EConsoleColor background_color)
{
    
    // If redirected, write to a file instead of console.
    DWORD dummy;
    if (stream.is_redirected)
    {
        if (stream.is_wide)
        {
            static wchar_t buf[LOG_BUFFER_SIZE];
            i32 required_size = MultiByteToWideChar(CP_UTF8, 0, message, -1, 0, 0) - 1;
            i32 offset;
            for (offset = 0; offset + LOG_BUFFER_SIZE , required_size; offset += LOG_BUFFER_SIZE)
            {
                // TODO(Matt): Little endian BOM.
                MultiByteToWideChar(CP_UTF8, 0, &message[offset], LOG_BUFFER_SIZE, buf, LOG_BUFFER_SIZE);
                WriteFile(stream.handle, buf, LOG_BUFFER_SIZE * 2, &dummy, 0);
            }
            i32 mod = required_size % LOG_BUFFER_SIZE;
            i32 size = MultiByteToWideChar(CP_UTF8, 0, &message[offset], mod, buf, LOG_BUFFER_SIZE) * 2;
            WriteFile(stream.handle, buf, size, &dummy, 0);
        }
        else
        {
            WriteFile(stream.handle, message, (DWORD)strlen(message), &dummy, 0);
        }
    }
    else
    {
        WORD attribute = Win32TranslateConsoleColors(text_color, background_color);
        SetConsoleTextAttribute(stream.handle, attribute);
        WriteConsole(stream.handle, message, (DWORD)strlen(message), &dummy, 0);
        attribute = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        SetConsoleTextAttribute(stream.handle, attribute);
    }
}

void PlatformPrintMessage(const char* message, EConsoleColor text_color, EConsoleColor background_color)
{
    // If we are in the debugger, output there.
    if (IsDebuggerPresent())
    {
        OutputDebugStringA(message);
        return;
    }
    
    // Otherwise, output to stdout.
    static Win32StandardStream stream = Win32GetStandardStream(STD_OUTPUT_HANDLE);
    Win32PrintToStream(message, stream, text_color, background_color);
}

void PlatformPrintError(const char* message, EConsoleColor text_color, EConsoleColor background_color)
{
    // If we are in the debugger, output there.
    if (IsDebuggerPresent())
    {
        OutputDebugStringA(message);
        return;
    }
    
    // Otherwise, output to stderr.
    static Win32StandardStream stream = Win32GetStandardStream(STD_ERROR_HANDLE);
    Win32PrintToStream(message, stream, text_color, background_color);
}

i32 __Win32FormatString(char *buff, i32 len, char *fmt, va_list list)
{
    // if a caller doesn't actually know the length of the
    // format list, and is querying for the required size,
    // attempt to format the string into the buffer first
    // before copying in the chars.
    //
    // This handles the case where the buffer is declared like:
    //     char *buff = nullptr;
    va_list cpy;
    va_copy(cpy, list);
    i32 needed_chars = vsnprintf(NULL, 0, fmt, cpy);
    va_end(cpy);
    
    if (needed_chars < len)
    {
        needed_chars = vsnprintf(buff, len, fmt, list);
    }
    
    return needed_chars;
}

void mprint(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    char *message = NULL;
    int chars_read = 1 + __Win32FormatString(message, 1, fmt, args);
    message = (char*)PlatformLocalAlloc(chars_read);
    __Win32FormatString(message, chars_read, fmt, args);
    
    va_end(args);
    
    PlatformPrintMessage(message);
}

void mprinte(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    char *message = NULL;
    int chars_read = 1 + __Win32FormatString(message, 1, fmt, args);
    message = (char*)PlatformLocalAlloc(chars_read);
    __Win32FormatString(message, chars_read, fmt, args);
    
    va_end(args);
    
    PlatformPrintError(message);
}