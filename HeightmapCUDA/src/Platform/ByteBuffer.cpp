
ByteBuffer ByteBuffer::Init(const char *filename, bool append, u64 expected_size)
{
    ByteBuffer result = {};
    result.ptr = 0;
    result.offset = 0;
    result.size = 0;
    
#if 0
    PlatformError err = PlatformOpenFile(&result.file, filename, true, append);
    if (err == PlatformError::Success)
    {
        result.ptr = (char*)MemAlloc(expected_size);;
        result.offset = result.ptr;
        result.size = expected_size;
    }
    else
    {
        // NOTE(Dustin): We have some problems and handle this
        result.ptr = NULL;
        result.offset = NULL;
        result.size = 0;
    }
#endif
    
    return result;
}

ByteBuffer ByteBuffer::Init(const char *filename)
{
    ByteBuffer result = {};
    result.ptr = 0;
    result.offset = 0;
    result.size = 0;
    
#if 0
    PlatformError err = PlatformReadFileToBuffer(filename, reinterpret_cast<u8**>(&result.ptr), (u32*)&result.size);
    if (err == PlatformError::Success)
    {
        result.offset = result.ptr;
    }
    else
    {
        mprinte("Failed to read file into ByteBuffer!\n");
        // NOTE(Dustin): We have some problems and handle this
        result.ptr = NULL;
        result.offset = NULL;
        result.size = 0;
    }
#endif
    
    return result;
}

void ByteBuffer::Destroy()
{
#if 0
    if (file >= 0) PlatformCloseFile(&file);
    if (ptr) MemFree(ptr);
#endif
    
    file   = INVALID_FILE_ID;
    ptr    = 0;
    offset = 0;
    size   = 0;
}

// Write API
void ByteBuffer::Write(void *data, u64 sizeof_data_to_write)
{
    if (sizeof_data_to_write + (offset - ptr) <= size)
    {
        memcpy(offset, data, sizeof_data_to_write);
        offset += sizeof_data_to_write;
    }
    else
    {
        mprinte("Hey, writing too much memory to the ByteBuffer.\n");
        mprinte("\tTrying to write %lld bytes.\n", sizeof_data_to_write);
        mprinte("\tOnly have %lld bytes left.\n", offset - ptr);
    }
}

void ByteBuffer::Write(const char *fmt, ...)
{
}

void ByteBuffer::Flush()
{
#if 0
    PlatformError err = PlatformWriteBufferToFile((u8*)ptr, offset - ptr, file);
    if (err != PlatformError::Success)
    {
        mprinte("Unable to flush the Byte Buffer to a file!\n");
    }
#endif
}

// Read API
void ByteBuffer::Read(void *data, u64 sizeof_data_to_read)
{
#if 0
    if (sizeof_data_to_read + (offset - ptr) <= size)
    {
        memcpy(data, offset, sizeof_data_to_read);
        offset += sizeof_data_to_read;
    }
    else
    {
        mprinte("Hey, reading too much memory to the ByteBuffer.\n");
        mprinte("\tTrying to read %lld bytes.\n", sizeof_data_to_read);
        mprinte("\tOnly have %lld bytes left.\n", offset - ptr);
    }
#endif
}
