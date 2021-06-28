
PlatformError PlatformCreateDirectory(const char *abs_path)
{
    PlatformError result = PlatformError::Success;
    
    BOOL err = CreateDirectoryA(abs_path, NULL);
    if (err == 0)
    {
        DWORD last_err = GetLastError();
        if (last_err == ERROR_ALREADY_EXISTS)
        {
            result = PlatformError::DirectoryAlreadyExists;
        }
        else if (last_err == ERROR_PATH_NOT_FOUND)
        {
            result = PlatformError::PathNotFound;
        }
    }
    
    return result;
}


PlatformError PlatformOpenFile(file_id *fd, const char *filename, bool is_write, bool append)
{
    PlatformError result = PlatformError::Success;
    Str long_path = PlatformNormalizePath(filename);
    HANDLE handle = 0;
    if (append)
    {
        *fd = CreateFileA(long_path, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
    }
    else
    {
        *fd = CreateFileA(long_path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
        
        //if (handle == INVALID_HANDLE_VALUE)
        //{
		//handle = CreateFileA(long_path, GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
        //}
    }
    if (*fd  == INVALID_HANDLE_VALUE) 
    {
        result = PlatformError::FileOpenFailure;
    }
    
    return result;
}

PlatformError PlatformCloseFile(file_id *id)
{
    CloseHandle(*id);
    return PlatformError::Success;
}

// TODO(Matt): Make a better string type.
PlatformError PlatformReadFileToBuffer(const char* file_path, u8** buffer, u32* size)
{
    PlatformError result = PlatformError::Success;
    
    *size = 0;
    *buffer = 0;
    HANDLE handle = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
    
    if (handle == INVALID_HANDLE_VALUE) 
    {
        PlatformPrintError("Unable to open file!\n");
        return PlatformError::FileOpenFailure;
    }
	
    u32 size_or_error = GetFileSize(handle, 0);
	if (size_or_error == INVALID_FILE_SIZE) 
    {
        PlatformPrintError("Unable to open file: size failure!\n");
        return PlatformError::FileOpenFailure;
    }
    else *size = size_or_error;
    
	*buffer = (u8*)MemAlloc(*size + 1);
    
	DWORD dummy;
    if (!ReadFile(handle, *buffer, *size, &dummy, 0))
    {
        PlatformPrintError("Unable to read file!\n");
        CloseHandle(handle);
        MemFree(*buffer);
		*size = 0;
        
        result = PlatformError::FileReadFailure;
    }
    CloseHandle(handle);
    
    (*buffer)[*size] = 0;
    
    return result;
}

PlatformError PlatformWriteBufferToFile(const char* file_path, u8* buffer, u64 size, bool append)
{
    PlatformError result = PlatformError::Success;
    Str long_path = {};
    long_path = PlatformNormalizePath(file_path);
    
    HANDLE handle = 0;
    if (append)
    {
        handle = CreateFileA(long_path, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
    }
    else
    {
        handle = CreateFileA(long_path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
        
        //if (handle == INVALID_HANDLE_VALUE)
        //{
		//handle = CreateFileA(long_path, GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
        //}
    }
    if (handle == INVALID_HANDLE_VALUE) return PlatformError::FileOpenFailure;
    
    DWORD dummy;
    u32 count = (u32)(size / U32_MAX);
    u32 mod = size % U32_MAX;
    for (u32 i = 0; i < count; ++i)
    {
        if (!WriteFile(handle, buffer + (u64)(i * U32_MAX), U32_MAX, &dummy, 0))
        {
            CloseHandle(handle);
            return PlatformError::FileWriteFailure;
        }
    }
    
    if (!WriteFile(handle, buffer + (u64)(U32_MAX * count), mod, &dummy, 0))
    {
        CloseHandle(handle);
        return PlatformError::FileWriteFailure;
    }
    CloseHandle(handle);
    return result;
}

Str PlatformGetFullExecutablePath()
{
    char buf[MAX_PATH];
    if (GetCurrentDirectory(MAX_PATH, buf) > MAX_PATH) return {};
    for (u32 i = 0; buf[i]; ++i) if (buf[i] == '\\') buf[i] = '/';
    return Str(buf);
}

Str PlatformNormalizePath(const char* path)
{
    Str result = {};
    
    // If the string is null or has length < 2, just return an empty one.
    if (!path || !path[0] || !path[1]) return result;
    
    // If a relative path, append it to the full executable path.
    if (path[0] == '/' || path[0] == '\\' || path[0] == '.') result = PlatformGetFullExecutablePath();
    if (path[0] == '.') result += '/';
    result += path;
    
    // Swap any back slashes for forward slashes.
    for (u32 i = 0; i < (u32)result.size; ++i) if (result[i] == '\\') result[i] = '/';
    
    // Strip double separators.
    for (u32 i = 0; i < (u32)result.size - 1; ++i)
    {
        if (result[i] == '/' && result[i + 1] == '/')
        {
            for (u32 j = i; j < (u32)result.size; ++j) result[j] = result[j + 1];
            --result.size;
            --i;
        }
    }
    
    // Evaluate any relative specifiers (./).
    if (result[0] == '.' && result[1] == '/')
    {
        for (u32 i = 0; i < (u32)result.size - 1; ++i) result[i] = result[i + 2];
        result.size -= 2;
    }
    for (u32 i = 0; i < (u32)result.size - 1; ++i)
    {
        if (result[i] != '.' && result[i + 1] == '.' && result[i + 2] == '/')
        {
            for (u32 j = i + 1; result[j + 1]; ++j)
            {
                result[j] = result[j + 2];
            }
            result.size -= 2;
        }
    }
    
    // Evaluate any parent specifiers (../).
    u32 last_separator = 0;
    for (u32 i = 0; (i < (u32)result.size - 1); ++i)
    {
        if (result[i] == '.' && result[i + 1] == '.' && result[i + 2] == '/')
        {
            u32 base = i + 2;
            u32 count = result.size - base;
            
            for (u32 j = 0; j <= count; ++j)
            {
                result[last_separator + j] = result[base + j];
            }
            
            result.size -= base - last_separator;
            i = last_separator;
            
            if (i > 0)
            {
                bool has_separator = false;
                for (i32 j = last_separator - 1; j >= 0; --j)
                {
                    if (result[j] == '/')
                    {
                        last_separator = j;
                        has_separator = true;
                        break;
                    }
                }
                if (!has_separator) return {};
            }
        }
        if (i > 0 && result[i - 1] == '/') last_separator = i - 1;
    }
    
    // Strip any leading or trailing separators.
    if (result[0] == '/')
    {
        for (u32 i = 0; i < (u32)result.size; ++i) result[i] = result[i + 1];
        --result.size;
    }
    
    if (result[result.size - 1] == '/')
    {
        result[result.size - 1] = '\0';
        --result.size;
    }
    return result;
}

static Str Win32ShowBasicFileDialog(i32 dialog_type, i32 resource_type)
{
	Str result = {};
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | 
								COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		switch (dialog_type)
		{
			case 0: // Open dialog
			{
				IFileOpenDialog *pFileOpen;
				
				// Create the FileOpenDialog object.
				hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, 
									  IID_IFileOpenDialog, (void**)&pFileOpen);
				
				if (SUCCEEDED(hr))
				{
					if (resource_type == 0)
					{
						COMDLG_FILTERSPEC rgSpec[] =
						{ 
							{ L"region file", L"*.region" }
						};
						pFileOpen->SetFileTypes(ARRAYCOUNT(rgSpec), rgSpec);
						pFileOpen->SetDefaultExtension(L"region");
					}
					else if (resource_type == 1)
					{
						COMDLG_FILTERSPEC rgSpec[] =
						{
							{ L"any image", L"*.png;*.bmp;*.jpg;*.jpeg" },
							{ L"png image", L"*.png" },
							{ L"bmp image", L"*.bmp" },
							{ L"jpeg image", L"*.jpg;*.jpeg" },
						};
						pFileOpen->SetFileTypes(ARRAYCOUNT(rgSpec), rgSpec);
						pFileOpen->SetDefaultExtension(L"png");
					}
					
					// Show the Open dialog box.
					hr = pFileOpen->Show(NULL);
					
					// Get the file name from the dialog box.
					if (SUCCEEDED(hr))
					{
						IShellItem *pItem;
						hr = pFileOpen->GetResult(&pItem);
						if (SUCCEEDED(hr))
						{
							PWSTR pszFilePath;
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
							
							// Display the file name to the user.
							if (SUCCEEDED(hr))
							{
								int size = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, 0, 0, 0, 0);
								result = Str8(size);
								WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, result, size, 0, 0);
								result[size] = 0;
								result.size = size;
								CoTaskMemFree(pszFilePath);
							}
							pItem->Release();
						}
					}
					pFileOpen->Release();
				}
			}
			break;
			case 1: // Save dialog
			{
				IFileSaveDialog *pFileSave;
				
				// Create the FileOpenDialog object.
				hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, 
									  IID_IFileSaveDialog, (void**)&pFileSave);
				
				if (SUCCEEDED(hr))
				{
					if (resource_type == 0)
					{
						COMDLG_FILTERSPEC rgSpec[] =
						{ 
							{ L"region file", L"*.region" }
						};
						pFileSave->SetFileTypes(ARRAYCOUNT(rgSpec), rgSpec);
						pFileSave->SetDefaultExtension(L"region");
					}
					else if (resource_type == 1)
					{
						COMDLG_FILTERSPEC rgSpec[] =
						{
							{ L"any image", L"*.png;*.bmp;*.jpg;*.jpeg" },
							{ L"png image", L"*.png" },
							{ L"bmp image", L"*.bmp" },
							{ L"jpeg image", L"*.jpg;*.jpeg" },
						};
						pFileSave->SetFileTypes(ARRAYCOUNT(rgSpec), rgSpec);
						pFileSave->SetDefaultExtension(L"png");
					}
					
					// Show the Open dialog box.
					hr = pFileSave->Show(NULL);
					
					// Get the file name from the dialog box.
					if (SUCCEEDED(hr))
					{
						IShellItem *pItem;
						hr = pFileSave->GetResult(&pItem);
						if (SUCCEEDED(hr))
						{
							PWSTR pszFilePath;
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
							
							// Display the file name to the user.
							if (SUCCEEDED(hr))
							{
								int size = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, 0, 0, 0, 0);
								result = Str8(size);
								WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, result, size, 0, 0);
								result[size] = 0;
								result.size = size;
								CoTaskMemFree(pszFilePath);
							}
							pItem->Release();
						}
					}
					pFileSave->Release();
				}
			}
			break;
			//default: Assert(false);
		}
		
		CoUninitialize();
	}
	return result;
}

Str PlatformShowBasicFileDialog(int type, int resource_type)
{
	return Win32ShowBasicFileDialog(type, resource_type);
}