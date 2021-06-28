#ifndef JENGINE_UTILS_JSTRING_H
#define JENGINE_UTILS_JSTRING_H

/*

jstring is a header-only string library that works as an alternative to
std::string. All strings are null-terminated.

jstring has 2 ways of storing the string: stack and heap allocated storage.
Small strings less than 12 bytes will be stored in the stack allocated storage.
In order to reduce the overall size of jstring, heap storage and static storage
are within a union. The size of the jstring is 16 bytes,

For heap storage string, they are originally allocated to be size + 1. Any
further resizes double the heap size.

It should be noted that a user should be careful when invoking the
equals operator using malloc or other custom allocation schemes. The new operator
invokes the default constructor (unless otherwise specified), however alternate
allocation schemes will not invoke a constructor. This is important in the following
case:

jstring *str = (jstring*)malloc(sizeof(jstring));
str = "test";

In this example, nothing neems amiss. "str" is allocated and then the operator= is
 invoked. However, if a user then does:

char *cstr = "other string";
str = cstr;

Again, the operator= is called to move "cstr" into "str". However, this will result
in a memory leak. This implementation of jstring will not free up the heap when
operator= is called. The reason for this is because if a heap allocation occurs
outside new, then the memory is not initialized, so if the implementation of the
operator= attempts to free any allocated memory, it will be impossible to tell if
the heap memory is actually being.

*/

union jstring
{
    struct {
        char sptr[12];
        unsigned len:31;
        unsigned heap:1; // 0 - heap is not used. 1 - heap is used
    };
    
    struct {
        char *hptr;
        unsigned reserved_heap_size;
        unsigned len:31;
        unsigned heap:1; // 0 - heap is not used. 1 - heap is used
    };
    
    jstring();
    jstring(const char *str);
    jstring(const char *str, unsigned size);
    // if legnth is greater than 11, then space
    // is reserved on the heap length+1
    jstring(unsigned req_heap_size);
    
    ~jstring();
    inline void Clear();
    
    jstring(jstring &other);
    jstring(jstring &&other);
    
    jstring& operator=(jstring &cpy);
    jstring& operator=(jstring &&cpy);
    jstring& operator=(const char *str);
    
    inline jstring& operator+=(const jstring &str);
    inline jstring& operator+=(const char *cstr);
    inline jstring& operator+=(char ch);
    
    // returns a pointer to the internal char array.
    // string is null terminated
    const char *GetCStr() const;
    
    unsigned GetLen() const;
    inline unsigned GetHeapSize() const;
    
    inline char operator[](int idx);
    bool operator==(const jstring &rhs) const;
};

//~
// USE WITH CARE - Copy on Write implementation
inline jstring operator+(const jstring &lhs, const jstring &str);
inline jstring operator+(const jstring &lhs, const char *cstr);
inline jstring operator+(const jstring &lhs, char ch);
inline jstring operator+(char ch, const jstring& rhs);

//~
// Preferred usage over jstring operator+
// rather than using COW, it takes in the reference of the returned
// value so a copy/move is not invoked upon return. If result already
// contains a string, the lhs and rhs is appended to that string. In other
// words, the original string is not overwritten.
void AddJString(jstring& result, const jstring &lhs, const jstring &rhs);
void AddJString(jstring &result, const char* lhs,    const jstring &rhs);
void AddJString(jstring &result, const jstring &lhs, const char* rhs);
void AddJString(jstring &result, const char* lhs,    const char* rhs);

//~
// Useful macros for creating jstrings
//------------------------------------------------
// NOTE(Dustin): Currently tstring implementation is identical to pstring
// I am keeping this function call for 2 reasons:
// 1. Backwards compatibility. tstring is used in a lot of places.
// 2. I might add back in temporary storage strings, so i would like to keep
//    that function call around just in case.
inline jstring pstring() {return jstring();}
inline jstring pstring(const char *p) {return jstring(p);}
inline jstring pstring(const char *p, unsigned s) {return jstring(p, s);}

inline jstring tstring() {return jstring();}
inline jstring tstring(const char *p) {return jstring(p);}
inline jstring tstring(const char *p, unsigned s) {return jstring(p, s);}

#endif //JSTRING_H

//~
#ifdef USE_JENGINE_JSTRING_IMPLEMENTATION

#if !defined(pstring_alloc) || !defined(pstring_realloc) || !defined(pstring_free)

#define pstring_alloc   malloc
#define pstring_realloc realloc
#define pstring_free    free

#endif

#if !defined(tstring_alloc) || !defined(tstring_realloc) || !defined(tstring_free)

#define tstring_alloc   malloc
#define tstring_realloc realloc
#define tstring_free    free

#endif

jstring::jstring()
: len(0)
, heap(0)
{
}

jstring::jstring(const char *str)
: len(0)
, heap(0)
{
    len = strlen(str);
    
    if (len > 11)
    {
        heap = 1;
        
        reserved_heap_size = len + 1;
        hptr = (char*)pstring_alloc(reserved_heap_size);
        
        memcpy(hptr, str, len);
        hptr[len] = 0;
    }
    else
    {
        memcpy(sptr, str, len);
        sptr[len] = 0;
    }
}

jstring::jstring(const char *str, unsigned size)
: len(0)
, heap(0)
{
    len = size;
    
    if (len > 11)
    {
        heap = 1;
        reserved_heap_size = len + 1;
        hptr = (char*)pstring_alloc(reserved_heap_size);
        
        memcpy(hptr, str, size);
        hptr[len] = 0;
    }
    else
    {
        memcpy(sptr, str, size);
        sptr[len] = 0;
    }
}

jstring::jstring(unsigned req_heap_size)
: len(0)
, heap(0)
{
    if (req_heap_size > 11) {
        heap = 1;
        reserved_heap_size = req_heap_size + 1;
        hptr = (char*)pstring_alloc(reserved_heap_size);
    }
}


jstring::~jstring()
{
    Clear();
}

inline void jstring::Clear()
{
    if (heap)
    {
        pstring_free(hptr);
        hptr = nullptr;
        reserved_heap_size = 0;
    }
    
    len = 0;
    heap = 0;
}

jstring::jstring(jstring &other)
: len(other.len)
, heap(other.heap)
{
    if (len > 11)
    {
        reserved_heap_size = other.reserved_heap_size;
        hptr = (char*)pstring_alloc(reserved_heap_size);
        
        memcpy(hptr, other.hptr, len);
        hptr[len] = 0;
    }
    else
    {
        memcpy(sptr, other.sptr, len);
        sptr[len] = 0;
    }
}

jstring::jstring(jstring &&other)
: len(other.len)
, heap(other.heap)
{
    if (len > 11)
    {
        reserved_heap_size = other.reserved_heap_size;
        hptr = other.hptr;
        
        // other no longer owns the hptr memory
        other.hptr = 0;
        other.reserved_heap_size = 0;
    }
    else
    {
        memcpy(sptr, other.sptr, len);
        sptr[len] = 0;
    }
    
    other.len  = 0;
    other.heap = 0;
}

jstring& jstring::operator=(jstring &other)
{
    //if (heap) pstring_free(hptr);
    
    len  = other.len;
    heap = other.heap;
    
    if (heap)
    {
        reserved_heap_size = other.reserved_heap_size;
        hptr = (char*)pstring_alloc(reserved_heap_size);
        
        memcpy(hptr, other.hptr, len);
        hptr[len] = 0;
    }
    else
    {
        memcpy(sptr, other.sptr, len);
        sptr[len] = 0;
    }
    
    return *this;
}

jstring& jstring::operator=(jstring &&other)
{
    //if (heap) pstring_free(hptr);
    
    len  = other.len;
    heap = other.heap;
    
    if (heap)
    {
        reserved_heap_size = other.reserved_heap_size;
        hptr = other.hptr;
        
        // other no longer owns the hptr memory
        other.hptr = 0;
        other.reserved_heap_size = 0;
    }
    else
    {
        memcpy(sptr, other.sptr, len);
        sptr[len] = 0;
    }
    
    other.len  = 0;
    other.heap = 0;
    
    return *this;
}

jstring& jstring::operator=(const char *str)
{
    if (heap) pstring_free(hptr);
    
    len  = strlen(str);
    heap = 0;
    
    if (len > 11)
    {
        reserved_heap_size = len + 1;
        hptr = (char*)pstring_alloc(reserved_heap_size);
        
        memcpy(hptr, str, len);
        hptr[len] = 0;
    }
    else
    {
        memcpy(sptr, str, len);
        sptr[len] = 0;
    }
    
    return *this;
}


inline jstring& jstring::operator+=(const jstring &str)
{
    /*
Conditions:

1. Both *this and str are heap alloc'd
 2. *this is heap alloc'd
3. str is heap alloc'd
4. Neither is heap alloc'd, but their sum need to be alloc'd
5. Neither is heap alloc'd, and their sum is less than 11

Confidently say:
- If at least one of them has a length greater 11, the result is heap alloc'd
- If their sum is less than 11, then neither are heap alloc'd

*/
    unsigned lhs_sz = len;
    unsigned rhs_sz = str.len;
    
    len += rhs_sz;
    
    if (heap)
    {
        if (len > reserved_heap_size)
        {
            reserved_heap_size = (reserved_heap_size * 2 > len) ? reserved_heap_size * 2 : len + 1;
            hptr = (char*)pstring_realloc(hptr, reserved_heap_size);
        }
        
        memcpy(hptr + lhs_sz, (str.heap) ? str.hptr : str.sptr, rhs_sz);
        hptr[len] = 0;
    }
    else if (len > 11)
    {
        heap = 1;
        reserved_heap_size = len + 1;
        hptr = (char*)pstring_alloc(reserved_heap_size);
        
        memcpy(hptr, sptr, lhs_sz);
        memcpy(hptr + lhs_sz, (str.heap) ? str.hptr : str.sptr, rhs_sz);
        hptr[len] = 0;
    }
    else
    {
        memcpy(hptr + lhs_sz, str.sptr, rhs_sz);
        sptr[len] = 0;
    }
    
    return *this;
}

// Null Terminated string
inline jstring& jstring::operator+=(const char *cstr)
{
    /*
Conditions:

1. *this is heap alloc'd
 4. *this is not heap alloc'd, but their sum needs to be alloc'd
5. *this is not heap alloc'd, and their sum is less than 11

Confidently say:
- If at least one of them has a length greater 11, the result is heap alloc'd
- If their sum is less than 11, then neither are heap alloc'd

*/
    unsigned lhs_sz = len;
    unsigned rhs_sz = (unsigned)strlen(cstr);
    
    len += rhs_sz;
    
    if (heap)
    {
        if (len > reserved_heap_size)
        {
            reserved_heap_size = (reserved_heap_size * 2 > len) ? reserved_heap_size * 2 : len + 1;
            hptr = (char*)pstring_realloc(hptr, reserved_heap_size);
        }
        
        memcpy(hptr + lhs_sz, cstr, rhs_sz);
    }
    else if (len > 11)
    {
        heap = 1;
        reserved_heap_size = len + 1;
        hptr = (char*)pstring_alloc(reserved_heap_size);
        
        memcpy(hptr, sptr, lhs_sz);
        memcpy(hptr + lhs_sz, cstr, rhs_sz);
    }
    else
    {
        memcpy(hptr + lhs_sz, cstr, rhs_sz);
    }
    
    return *this;
}

inline jstring& jstring::operator+=(char ch)
{
    len++;
    
    if (heap)
    {
        if (len > reserved_heap_size)
        {
            reserved_heap_size = (reserved_heap_size * 2 > len) ? reserved_heap_size * 2 : len + 1;
            hptr = (char*)pstring_realloc(hptr, reserved_heap_size);
        }
        
        hptr[len-1] = ch;
        hptr[len]   = 0;
    }
    else if (len > 11)
    {
        heap = 1;
        reserved_heap_size = len + 1;
        hptr = (char*)pstring_alloc(reserved_heap_size);
        
        hptr[len-1] = ch;
        hptr[len]   = 0;
    }
    else
    {
        sptr[len-1] = ch;
        sptr[len]   = 0;
    }
    
    return *this;
}


//-----------------------------------------
// USE WITH CARE - Copy on Write implementation
inline jstring operator+(const jstring &lhs, const jstring &rhs)
{
    jstring result = jstring(lhs.len + rhs.len);
    
    if (result.heap)
    {
        memcpy(result.hptr, (lhs.heap) ? lhs.hptr : lhs.sptr, lhs.len);
        memcpy(result.hptr + lhs.len, (rhs.heap) ? rhs.hptr : rhs.sptr, rhs.len);
    }
    else
    {
        memcpy(result.hptr, lhs.sptr, lhs.len);
        memcpy(result.hptr + lhs.len, rhs.sptr, rhs.len);
    }
    
    return result;
}

inline jstring operator+(const jstring &lhs, const char *cstr)
{
    unsigned str_len = (unsigned)strlen(cstr);
    
    jstring result = jstring(lhs.len + str_len);
    
    if (result.heap)
    {
        memcpy(result.hptr, (lhs.heap) ? lhs.hptr : lhs.sptr, lhs.len);
        memcpy(result.hptr + lhs.len, cstr , str_len);
    }
    else
    {
        memcpy(result.hptr, lhs.sptr, lhs.len);
        memcpy(result.hptr + lhs.len, cstr , str_len);
    }
    
    return result;
}

inline jstring operator+(const jstring& lhs, char ch)
{
    jstring result = jstring(lhs.len + 1);
    result.len = lhs.len + 1;
    
    if (result.heap)
    {
        memcpy(result.hptr, (lhs.heap) ? lhs.hptr : lhs.sptr, lhs.len);
        result.hptr[result.len-1] = ch;
        result.hptr[result.len]   = 0;
    }
    else
    {
        memcpy(result.sptr, lhs.sptr, lhs.len);
        result.sptr[result.len-1] = ch;
        result.sptr[result.len]   = 0;
    }
    
    return result;
}

inline jstring operator+(char ch, const jstring& rhs)
{
    jstring result = jstring(rhs.len + 1);
    result.len = rhs.len + 1;
    
    if (result.heap)
    {
        result.hptr[0] = ch;
        memcpy(result.hptr + 1, (rhs.heap) ? rhs.hptr : rhs.sptr, rhs.len);
        result.hptr[result.len] = 0;
    }
    else
    {
        result.sptr[0] = ch;
        memcpy(result.sptr + 1, rhs.sptr, rhs.len);
        result.sptr[result.len] = 0;
    }
    
    return result;
}

//-----------------------------------------

void AddJString(jstring &result, const jstring &lhs, const jstring &rhs)
{
    unsigned add_len = result.len + lhs.len + rhs.len;
    
    if (result.heap)
    {
        if (result.reserved_heap_size <= add_len)
        {
            result.reserved_heap_size = (result.reserved_heap_size * 2 > add_len) ? result.reserved_heap_size * 2 : add_len + 1;
            result.hptr = (char*)pstring_realloc(result.hptr, result.len);
        }
        
        memcpy(result.hptr + result.len, (lhs.heap) ? lhs.hptr : lhs.sptr, lhs.len);
        memcpy(result.hptr + result.len + lhs.len, (rhs.heap) ? rhs.hptr : rhs.sptr, rhs.len);
        result.hptr[add_len] = 0;
    }
    else if (add_len > 11)
    {
        result.heap = 1;
        result.reserved_heap_size = add_len + 1;
        result.hptr = (char*)pstring_alloc(result.reserved_heap_size);
        
        memcpy(result.hptr, result.sptr, result.len);
        memcpy(result.hptr + result.len, (lhs.heap) ? lhs.hptr : lhs.sptr, lhs.len);
        memcpy(result.hptr + result.len + lhs.len, (rhs.heap) ? rhs.hptr : rhs.sptr, rhs.len);
        result.hptr[add_len] = 0;
    }
    else
    {
        memcpy(result.sptr + result.len, lhs.sptr, lhs.len);
        memcpy(result.sptr + result.len + lhs.len, rhs.sptr, rhs.len);
        result.sptr[add_len] = 0;
    }
    
    result.len = add_len;
}

void AddJString(jstring &result, const char* lhs, const jstring &rhs)
{
    unsigned str_len = (unsigned)strlen(lhs);
    unsigned add_len = result.len + str_len + rhs.len;
    
    if (result.heap)
    {
        if (result.reserved_heap_size <= add_len)
        {
            result.reserved_heap_size = (result.reserved_heap_size * 2 > add_len) ? result.reserved_heap_size * 2 : add_len + 1;
            result.hptr = (char*)pstring_realloc(result.hptr, result.len);
        }
        
        memcpy(result.hptr + result.len, lhs, str_len);
        memcpy(result.hptr + result.len + str_len, (rhs.heap) ? rhs.hptr : rhs.sptr, rhs.len);
        result.hptr[add_len] = 0;
    }
    else if (add_len > 11)
    {
        result.heap = 1;
        result.reserved_heap_size = add_len + 1;
        result.hptr = (char*)pstring_alloc(result.reserved_heap_size);
        
        memcpy(result.hptr, result.sptr, result.len);
        memcpy(result.hptr + result.len, lhs, str_len);
        memcpy(result.hptr + result.len + str_len, (rhs.heap) ? rhs.hptr : rhs.sptr, rhs.len);
        result.hptr[add_len] = 0;
    }
    else
    {
        memcpy(result.sptr + result.len, lhs, str_len);
        memcpy(result.sptr + result.len + str_len, rhs.sptr, rhs.len);
        result.sptr[add_len] = 0;
    }
    
    result.len = add_len;
}

void AddJString(jstring &result, const jstring &lhs, const char* rhs)
{
    unsigned str_len = (unsigned)strlen(rhs);
    unsigned add_len = result.len + str_len + lhs.len;
    
    if (result.heap)
    {
        if (result.reserved_heap_size <= add_len)
        {
            result.reserved_heap_size = (result.reserved_heap_size * 2 > add_len) ? result.reserved_heap_size * 2 : add_len + 1;
            result.hptr = (char*)pstring_realloc(result.hptr, result.len);
        }
        
        memcpy(result.hptr + result.len, (lhs.heap) ? lhs.hptr : lhs.sptr, lhs.len);
        memcpy(result.hptr + result.len + lhs.len, rhs, str_len);
        result.hptr[add_len] = 0;
    }
    else if (add_len > 11)
    {
        result.heap = 1;
        result.reserved_heap_size = add_len + 1;
        result.hptr = (char*)pstring_alloc(result.reserved_heap_size);
        
        memcpy(result.hptr, result.sptr, result.len);
        memcpy(result.hptr + result.len, (lhs.heap) ? lhs.hptr : lhs.sptr, lhs.len);
        memcpy(result.hptr + result.len + lhs.len, rhs, str_len);
        result.hptr[add_len] = 0;
    }
    else
    {
        memcpy(result.sptr + result.len, lhs.sptr, lhs.len);
        memcpy(result.sptr + result.len + lhs.len, rhs, str_len);
        result.sptr[add_len] = 0;
    }
    
    result.len = add_len;
}

void AddJString(jstring &result, const char* lhs, const char* rhs)
{
    unsigned lhs_sz = (unsigned)strlen(lhs);
    unsigned rhs_sz = (unsigned)strlen(rhs);
    unsigned add_len = result.len + lhs_sz + rhs_sz;
    
    if (result.heap)
    {
        if (result.reserved_heap_size <= add_len)
        {
            result.reserved_heap_size = (result.reserved_heap_size * 2 > add_len) ? result.reserved_heap_size * 2 : add_len + 1;
            result.hptr = (char*)pstring_realloc(result.hptr, result.len);
        }
        
        memcpy(result.hptr + result.len, lhs, lhs_sz);
        memcpy(result.hptr + result.len + lhs_sz, rhs, rhs_sz);
        result.hptr[add_len] = 0;
    }
    else if (add_len > 11)
    {
        result.heap = 1;
        result.reserved_heap_size = add_len + 1;
        result.hptr = (char*)pstring_alloc(result.reserved_heap_size);
        
        memcpy(result.hptr, result.sptr, result.len);
        memcpy(result.hptr + result.len, lhs, lhs_sz);
        memcpy(result.hptr + result.len + lhs_sz, rhs, rhs_sz);
        result.hptr[add_len] = 0;
    }
    else
    {
        memcpy(result.sptr + result.len, lhs, lhs_sz);
        memcpy(result.sptr + result.len + lhs_sz, rhs, rhs_sz);
        result.sptr[add_len] = 0;
    }
}

const char* jstring::GetCStr() const
{
    const char *result = nullptr;
    
    // NOTE(Dustin): Redundant if statement?
    if (heap) result = hptr;
    else      result = sptr;
    
    return result;
}

unsigned jstring::GetLen() const
{
    return len;
}

inline unsigned jstring::GetHeapSize() const
{
    unsigned result;
    
    if (heap) result = reserved_heap_size;
    else      result = 0;
    
    return result;
}


inline char jstring::operator[](int idx)
{
    char result;
    
    if (heap) result = hptr[idx];
    else      result = sptr[idx];
    
    return result;
}

bool jstring::operator==(const jstring &rhs) const
{
    struct U128 {
        struct { u64 upper, lower; };
        
        u64 bits[2];
    };
    
    U128 lhs_hash = {};
    U128 rhs_hash = {};
    
    Hash128((heap) ? hptr : sptr, len, &lhs_hash);
    Hash128((rhs.heap) ? rhs.hptr : rhs.sptr, rhs.len, &rhs_hash);
    
    return (lhs_hash.upper == rhs_hash.upper) && (lhs_hash.lower == rhs_hash.lower);
}

#endif