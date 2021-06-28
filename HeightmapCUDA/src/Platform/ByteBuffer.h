#ifndef _BYTE_BUFFER_H
#define _BYTE_BUFFER_H

struct ByteBuffer
{
    file_id file;
    char   *ptr;
    char   *offset;
    u64     size;
    
    // Open for Write initializer
    static ByteBuffer Init(const char *filename, bool append, u64 expected_sizeb);
    // Open for Read initializer
    static ByteBuffer Init(const char *filename);
    void Destroy();
    
    // Write API
    void Write(void *data, u64 sizeof_data_to_write);
    void Write(const char *fmt, ...);
    void Flush();
    
    // Read API
    void Read(void *data, u64 sizeof_data_to_write);
};

#endif //_BYTE_BUFFER_H
