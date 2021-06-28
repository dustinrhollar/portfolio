
#ifndef _UTIL_STRING_H
#define _UTIL_STRING_H

#define STRING_STACK_SIZE 12

typedef union 
{
    struct {
        char     sptr[STRING_STACK_SIZE];
        unsigned len:31;
        unsigned heap:1; // 0 - heap is not used. 1 - heap is used
    };

    struct {
        char*    hptr;
        unsigned heap_size;
    };
} Str;

// If a user only wants to init the string with a CAP, then
// pass NULL as ptr. 
void str_init(Str *str, const char *ptr, u32 len);
void str_free(Str *str);
void str_set_cap(Str *str, u32 len);
const char* str_to_string(Str *str);
void str_add(Str *result, Str *left, Str *right);
void str_concat(Str *left, Str *right);

void str_log(Str *str);

#endif // _STRING_H


#if defined(MAPLE_STRING_IMPLEMENTATION)

void str_init(Str *str, const char *ptr, u32 len)
{
    str->len = 0;
    str->heap = 0;

    if (len < STRING_STACK_SIZE)
    {
        if (ptr)
        {
            memcpy(str->sptr, ptr, len);
            str->sptr[len] = 0;
            str->len = len;
        }
    }
    else
    {
        str->heap = 1;
        str->heap_size = len+1;
        str->hptr = (char*)MemAlloc(len+1);
        
        if (ptr)
        {
            str->len = len;
            memcpy(str->hptr, ptr, len);
            str->hptr[len] = 0;
        }
    }
}

void str_free(Str *str)
{
    if (str->len > 0 && str->heap == 1) MemFree(str->hptr);
    str->len  = 0;
    str->heap = 0;
    str->hptr = 0;
}

void str_set_cap(Str *str, u32 len)
{
    // Only think about adjusting the cap if the length
    // is greater than the stack size
    if (len >= STRING_STACK_SIZE)
    {
        // Don't bother allocating new space if the heap
        // is already large enough
        if (str->heap == 1 && len > str->heap_size)
        {
            u32 new_heap = len + 1;
            char *tmp = MemAlloc(new_heap);
            memcpy(tmp, str->hptr, str->len);
            tmp[str->len] = 0;
            MemFree(str->hptr);
            str->hptr = tmp;
            str->heap_size = new_heap;
        }
        else
        {
            u32 new_heap = len + 1;
            char *tmp = MemAlloc(new_heap);
            memcpy(tmp, str->sptr, str->len);
            tmp[str->len] = 0;
            MemFree(str->hptr);
            str->hptr = tmp;
            str->heap_size = new_heap;
        }
        str->heap = 1;
    }
}

const char* str_to_string(Str *str)
{
    if (str->heap) return str->hptr;
    else           return str->sptr;
}

void str_log(Str *str)
{
    LogInfo("%s", str_to_string(str));
}

void str_add(Str *result, Str *left, Str *right)
{
    str_init(result, NULL, left->len + right->len);

    if (result->heap)
    {
        LogDebug("Adding the two string: %s : %s. The result should allocate on the heap. %d", 
                str_to_string(left),
                str_to_string(right),
                result->len);

        memcpy(result->hptr, str_to_string(left), left->len);
        result->len += left->len;
        memcpy(result->hptr+result->len, str_to_string(right), right->len);   
        result->len += right->len;
        result->hptr[result->len] = 0;
    }
    else
    {
        memcpy(result->sptr, str_to_string(left), left->len);
        result->len += left->len;
        memcpy(result->sptr+result->len, str_to_string(right), right->len);   
        result->len += right->len;
        result->sptr[result->len] = 0;
    }
}

void str_concat(Str *left, Str *right)
{
    str_set_cap(left, left->len + right->len);
    if (left->heap)
    {
        memcpy(left->hptr+left->len, str_to_string(right), right->len);
        left->len += right->len;
        left->hptr[left->len] = 0;
    }
    else
    {
        memcpy(left->sptr+left->len, str_to_string(right), right->len);
        left->len += right->len;
        left->sptr[left->len] = 0;
    }
}

#endif //
