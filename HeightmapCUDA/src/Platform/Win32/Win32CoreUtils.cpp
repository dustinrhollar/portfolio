
MAPLE_GUID PlatformGenerateGuid()
{
    GUID result;
    HRESULT err = CoCreateGuid(&result);
    return result;
}

Str PlatformGuidToString(MAPLE_GUID guid)
{
    i32 req = snprintf(NULL, 0, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX", 
                       guid.Data1, guid.Data2, guid.Data3, 
                       guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                       guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    
    Str result = Str8(req);
    req = snprintf(result.ptr, req, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX", 
                   guid.Data1, guid.Data2, guid.Data3, 
                   guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                   guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    result.ptr[req] = 0;
    result.size = req;
    
    return result;
}

MAPLE_GUID PlatformStringToGuid(const char* guid_str)
{
    MAPLE_GUID result = {};
    
    int res = sscanf(guid_str, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
                     &result.Data1, &result.Data2, &result.Data3, 
                     &result.Data4[0], &result.Data4[1], &result.Data4[2], &result.Data4[3],
                     &result.Data4[4], &result.Data4[5], &result.Data4[6], &result.Data4[7]);
    
    return result;
}

bool PlatformShowAssertDialog(const char* message, const char* file, u32 line)
{
	const size_t output_buffer_size = 4096;
    char buf[output_buffer_size];
	snprintf(buf, output_buffer_size,
			 "Assertion Failed!\n"
			 "    File: %s\n"
			 "    Line: %u\n"
			 "    Statement: ASSERT(%s)\n",
			 file, line, message);
	PlatformPrintError(buf);
    snprintf(buf, output_buffer_size,
			 "--File--\n"
			 "%s\n"
			 "\n"
			 "Line %u\n"
			 "\n"
			 "--Statement--\n"
			 "ASSERT(%s)\n"
			 "\n"
			 "Press Abort to stop execution, Retry to set a breakpoint (if debugging), or Ignore to continue execution.\n", file, line, message);
	
	int result = MessageBoxA(0, buf, "Assertion Failed!", MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
	if (result == IDABORT) exit(0);
	else if (result == IDRETRY) return true;
	else return false;
}

void PlatformShowErrorDialog(const char* message)
{
    MessageBoxA(0, message, "Error!", MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
}

void Win32ShowErrorDialog(const char* message)
{
    MessageBoxA(0, message, "Error!", MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
}

void PlatformFatalError(const char* message, ...)
{
    const size_t output_buffer_size = 4096;
    char output_buffer[output_buffer_size];
    va_list args;
    va_start (args, message);
    vsnprintf(output_buffer, output_buffer_size, message, args);
    Win32ShowErrorDialog(output_buffer);
    va_end(args);
    exit(-1);
}



void* Win32RequestMemory(u64 Size)
{
    SYSTEM_INFO sSysInfo;
    DWORD       dwPageSize;
    LPVOID      lpvBase;
    u64         ActualSize;
    
    GetSystemInfo(&sSysInfo);
    dwPageSize = sSysInfo.dwPageSize;
    
    ActualSize = (Size + (u64)dwPageSize - 1) & ~((u64)dwPageSize - 1);
    
    lpvBase = VirtualAlloc(NULL,                    // System selects address
                           ActualSize,              // Size of allocation
                           MEM_COMMIT|MEM_RESERVE,  // Allocate reserved pages
                           PAGE_READWRITE);          // Protection = no access
    
    return lpvBase;
}

void Win32ReleaseMemory(void *Ptr, u64 Size)
{
    BOOL bSuccess = VirtualFree(Ptr,           // Base address of block
                                0,             // Bytes of committed pages
                                MEM_RELEASE);  // Decommit the pages
    
    if (!bSuccess)
    {
        DWORD error = GetLastError();
        PlatformFatalError("Unable to free a VirtualAlloc allocation!\n\tError: %ld\n", error);
    }
}

u32 PlatformClz(u32 Value)
{
    unsigned long LeadingZero = 0;
    
    if (_BitScanReverse64(&LeadingZero, Value))
        return 31 - LeadingZero;
    else
        return 32;
}

u32 PlatformCtz(u32 Value)
{
    unsigned long TrailingZero = 0;
    
    if (Value == 0) return 0;
    else if (_BitScanForward64(&TrailingZero, Value))
        return TrailingZero;
    else
        return 32;
}

u32 PlatformCtzl(u64 Value)
{
    unsigned long TrailingZero = 0;
    
    if (_BitScanForward64(&TrailingZero, Value))
        return TrailingZero;
    else
        return 32;
}

u32 PlatformClzl(u64 Value)
{
    unsigned long LeadingZero = 0;
    
    if (_BitScanReverse64(&LeadingZero, Value))
        return 31 - LeadingZero;
    else
        return 32;
}