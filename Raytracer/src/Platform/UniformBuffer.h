#ifndef _UNIFORM_BUFFER_H
#define _UNIFORM_BUFFER_H

typedef struct UniformBuffer
{
    char *p;
    char *o;
    u64   size;
    u32   stride;
} UniformBuffer;

void ub_init(UniformBuffer *buffer, u32 count, u32 stride);
void ub_free(UniformBuffer *buffer);
void ub_ins(UniformBuffer *buffer, void *elements, u32 count);
void ub_copy(UniformBuffer *dst, UniformBuffer *src);

#endif //_UNIFORM_BUFFR_H
