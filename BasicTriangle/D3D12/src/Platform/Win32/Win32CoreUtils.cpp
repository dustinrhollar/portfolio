
struct Win32ProcessorInfo
{
    DWORD logical_processor_count = 0;
    DWORD physical_core_count = 0;
    DWORD thread_per_processor = 0;
};

void 
Win32GetProcessorInfo(Win32ProcessorInfo *info)
{
    typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,PDWORD);
    
    // Helper lambda to count set bits in the processor mask.
    auto Win32CountSetBits = [&](ULONG_PTR bitMask) -> DWORD {
        DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
        DWORD bitSetCount = 0;
        ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
        DWORD i;
        
        for (i = 0; i <= LSHIFT; ++i)
        {
            bitSetCount += ((bitMask & bitTest)?1:0);
            bitTest/=2;
        }
        
        return bitSetCount;
    };
    
    LPFN_GLPI glpi;
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;
    DWORD logicalProcessorCount = 0;
    DWORD processorCoreCount = 0;
    DWORD byteOffset = 0;
    
    glpi = (LPFN_GLPI) GetProcAddress(GetModuleHandle(TEXT("kernel32")),
                                      "GetLogicalProcessorInformation");
    if (NULL == glpi) 
    {
        LogError("GetLogicalProcessorInformation is not supported.");
    }
    else
    {
        while (!done)
        {
            DWORD rc = glpi(buffer, &returnLength);
            if (FALSE == rc) 
            {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
                {
                    if (buffer) 
                        free(buffer);
                    
                    buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
                                                                           returnLength);
                    
                    if (NULL == buffer) 
                    {
                        LogFatal("Error: Allocation failure");
                    }
                } 
                else 
                {
                    LogFatal("Error %d", GetLastError());
                }
            } 
            else
            {
                done = TRUE;
            }
        }
        
        ptr = buffer;
        
        while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) 
        {
            switch (ptr->Relationship) 
            {
                case RelationProcessorCore:
                {
                    processorCoreCount++;
                    // A hyperthreaded core supplies more than one logical processor.
                    logicalProcessorCount += Win32CountSetBits(ptr->ProcessorMask);
                } break;
            }
            
            byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
            ptr++;
        }
        
#if 0
        LogInfo("Number of processor cores:         %d", processorCoreCount);
        LogInfo("Number of logical processor cores: %d", logicalProcessorCount);
        LogInfo("Number of threads per processor:   %d", logicalProcessorCount / processorCoreCount);
#endif
        
        info->physical_core_count     = processorCoreCount;
        info->logical_processor_count = logicalProcessorCount; 
        info->thread_per_processor    = logicalProcessorCount / processorCoreCount;
    }
}

MAPLE_GUID 
PlatformGenerateGuid()
{
    GUID result;
    HRESULT err = CoCreateGuid(&result);
    return result;
}

Str 
PlatformGuidToString(MAPLE_GUID guid)
{
    i32 req = snprintf(NULL, 0, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX", 
                       guid.Data1, guid.Data2, guid.Data3, 
                       guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                       guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    
    Str result;
    str_init(&result, 0, 0);
    str_set_cap(&result, req);
    
    req = snprintf(str_to_string(&result), req, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX", 
                   guid.Data1, guid.Data2, guid.Data3, 
                   guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                   guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    
    char *s = str_to_string(&result);
    s[req] = 0;
    result.len = req;
    
    return result;
}

MAPLE_GUID 
PlatformStringToGuid(const char* guid_str)
{
    MAPLE_GUID result;
    
    int res = sscanf_s(guid_str, "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
                       &result.Data1, &result.Data2, &result.Data3, 
                       &result.Data4[0], &result.Data4[1], &result.Data4[2], &result.Data4[3],
                       &result.Data4[4], &result.Data4[5], &result.Data4[6], &result.Data4[7]);
    
    return result;
}

void* 
PlatformAlloc(u64 Size)
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

void 
PlatformFree(void *Ptr)
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

u32 
PlatformClz(u32 Value)
{
    unsigned long LeadingZero = 0;
    
    if (_BitScanReverse64(&LeadingZero, Value))
        return 31 - LeadingZero;
    else
        return 32;
}

u32 
PlatformCtz(u32 Value)
{
    unsigned long TrailingZero = 0;
    
    if (Value == 0) return 0;
    else if (_BitScanForward64(&TrailingZero, Value))
        return TrailingZero;
    else
        return 32;
}

u32 
PlatformCtzl(u64 Value)
{
    unsigned long TrailingZero = 0;
    
    if (_BitScanForward64(&TrailingZero, Value))
        return TrailingZero;
    else
        return 32;
}

u32 
PlatformClzl(u64 Value)
{
    unsigned long LeadingZero = 0;
    
    if (_BitScanReverse64(&LeadingZero, Value))
        return 31 - LeadingZero;
    else
        return 32;
}