
void ub_init(UniformBuffer *buffer, u32 count, u32 stride)
{
    buffer->size = (count) * stride;
    buffer->stride = stride;
    buffer->p = (char*)MemAlloc(buffer->size);
    buffer->o = buffer->p;
}

void ub_free(UniformBuffer *buffer)
{
    if (buffer->p) MemFree(buffer->p);
    buffer->o = 0;
    buffer->size = 0;
    buffer->stride = 0;
}

void ub_ins(UniformBuffer *buffer, void *elements, u32 count)
{
    u32 ins_size = count * buffer->stride;
    u64 open_sz = buffer->size - (buffer->o - buffer->p + ins_size);
    if (open_sz >= 0)
    {
        memcpy(buffer->o, elements, ins_size);
        buffer->o += ins_size;
    }
    else
    {
        LogError("UniformBuffer::Error: Tried to copy into Uniform Buffer that was too small!");
        LogError("UniformBuffer::Error: Error Info: OpenSize: %d!", open_sz);
    }
}

void ub_copy(UniformBuffer *dst, UniformBuffer *src)
{
    u64 copy_sz = src->o - src->p;
    u64 open_sz = dst->size - (dst->o - dst->p);
    if (copy_sz <= open_sz)
    {
        memcpy(dst->o, src->p, copy_sz);
        dst->o += copy_sz;
    }
    else
    {
        LogError("UniformBuffer::Error::Copy: Attempted to copy to a dst not big enough!");
    }
}
