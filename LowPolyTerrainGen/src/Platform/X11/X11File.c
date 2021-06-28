u32 X11GetExeFilepath(char *buf, u32 buf_len)
{
    u32 result;
    char exe_filename[PATH_MAX];
    ssize_t count = readlink( "/proc/self/exe", exe_filename, PATH_MAX );
    exe_filename[count] = 0;
    
    if (count > 0)
    {
        char *last_slash = 0;
        for (char *scan = exe_filename; *scan; ++scan)
        {
            if (*scan == '/') last_slash = scan;
        }

        result = last_slash - exe_filename;
        if (buf_len < result)
        {
            LogError("Attempted to retrieve EXE path with buffer not big enough.");
            result = buf_len - 1;
        }

        memcpy(buf, exe_filename, result);
        buf[result] = 0;
    }
    else
    {
        LogError("Failed to get the EXE path!");
    }

    return result;
}

static u32 X11BuildAbsolutePath(char *buf, u32 size, u32 offset, const char *filename, u32 filename_len)
{
    u32 offset_orig = offset;

    u32 exe_len = X11GetExeFilepath(buf+offset, size);
    offset += exe_len;
    buf[offset++] = '/';
    
    memcpy(buf+offset, filename, filename_len);
    offset += filename_len;
    buf[offset] = 0;

    return offset - offset_orig;
}

PlatformErrorType PlatformReadFileToBuffer(const char* file_path, u8** buffer, u32* size)
{
    char scratch[PATH_MAX];
    u32 scratch_len = 0;

    PlatformErrorType result = PlatformError_Success;

    // Build the fullpath for the file
    scratch_len = X11BuildAbsolutePath(scratch, PATH_MAX, scratch_len, file_path, strlen(file_path));

    // Deterime file size
    struct stat file_info;
    int stat_ret = stat(scratch, &file_info);
    if (stat_ret == -1)
    {
        LogError("Failed to get file info for file: %s. Error: %s.", scratch, strerror(errno));
        return PlatformError_FileOpenFailure;
    }

    *size = stat_ret = file_info.st_size;

    // open the file
    int fd = open(scratch, O_RDONLY);
    if (fd < 0) 
    {
        LogError("Failed to open file: %s! Error: %s.", scratch, strerror(errno));
        return PlatformError_FileOpenFailure;
    }

    *buffer = (u8*)MemAlloc(*size + 1);
    ssize_t read_bytes = read(fd, *buffer, *size);
    if (read_bytes != *size)
    {
        LogError("Failure reading file: %s!", scratch);
        result = PlatformError_FileReadFailure;
    }

    (*buffer)[*size] = 0;

    close(fd);
    return result;
}

PlatformErrorType PlatformWriteBufferToFile(const char* file_path, u8* buffer, u64 size, bool append)
{
    char scratch[PATH_MAX];
    u32 scratch_len = 0;

    PlatformErrorType result = PlatformError_Success;

    // Build the fullpath for the file
    scratch_len = X11BuildAbsolutePath(scratch, PATH_MAX, scratch_len, file_path, strlen(file_path));
    
    int fd = open(scratch, O_WRONLY|O_TRUNC, S_IRWXU);
    if (fd < 0)
    {
        fd = open(scratch, O_WRONLY|O_CREAT, S_IRWXU);
        
        if (fd < 0)
        {
            LogError("Unable to create file: %s! Error: %s", scratch, strerror(errno));
            return PlatformError_FileOpenFailure; 
        }
    }
    
    if (write(fd, buffer, size) < 0)
    {
        LogError("Failed to write to file %s. Error: %s", scratch, strerror(errno));
        result = PlatformError_FileWriteFailure;
    }

    close(fd);

    return result;
}
