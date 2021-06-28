#ifndef _USTRING_H
#define _USTRING_H
struct Str8;
struct Str16;
typedef Str8 Str;

int SizeInUTF8(char32_t cp);
int SizeInUTF16(char32_t cp);
int SizeFromLeading(char c);
int SizeFromLeading(char16_t c);
char32_t ToCodePoint(const char utf8[4], int* consumed);
char32_t ToCodePoint(const char16_t utf16[2], int* consumed);
bool ToUTF8(char32_t cp, char out[4], int* size);
bool ToUTF16(char32_t cp, char16_t out[2], int* size);

// Byte array, can hold ANSI or UTF-8.
// TODO(Matt): Add splice/remove, contains char/substring, etc.
// TODO(Matt): UTF8 Length.
struct Str8
{
    char* ptr; // Buffer pointer. Allocated with USTRING_MALLOC, USTRING_REALLOC, and USTRING_FREE.
    int size; // Number of bytes used by the string, *excluding* the null terminator.
    int capacity; // Number of bytes reserved by the string, must be greater than size.
    
    Str8() : ptr(0), size(0), capacity(0) {};
    Str8(int initial_reserve);
    Str8(const char* str);
    Str8(const char* str, int length);
    Str8(const Str8& str);
    Str8(Str8&& str);
    Str8(const char16_t* str);
    Str8(const char16_t* str, int length);
    Str8(const Str16& str);
    Str8(const char32_t* str);
    Str8(const char32_t* str, int length);
    ~Str8();
    
    // Copy-assignment and move-assignment.
    inline Str8& operator=(const Str8& str);
    inline Str8& operator=(Str8&& str);
    
    inline operator char*() const {return ptr;} // Auto-cast to char*.
    
    // Array index operators.
    inline char& operator[](int i) {return ptr[i];}
    inline const char& operator[](int i) const {return ptr[i];}
    
    // Comparison operators.
    inline bool operator==(const Str8& str);
    inline bool operator==(const char* str);
    
    // Append string, C-string, or a single character.
    inline Str8& operator+=(const Str8& str);
    inline Str8& operator+=(const char* str);
    inline Str8& operator+=(char c);
    // TODO(Matt): Append UTF-16 string.
    // TODO(Matt): Append Codepoint buffer.
    // TODO(Matt): Append UTF-16 character.
    // TODO(Matt): Append codepoint.
    // TODO(Matt): Set reserve size.
};

// More append combinations.
inline Str8 operator+(const Str8& a, const Str8& b);
inline Str8 operator+(const Str8& a, const char* b);
inline Str8 operator+(const char* a, const Str8& b);
inline Str8 operator+(const Str8& a, char b);
inline Str8 operator+(char a, const Str8& b);
// TODO(Matt): Append UTF-16 string.
// TODO(Matt): Append Codepoint buffer.
// TODO(Matt): Append UTF-16 character.
// TODO(Matt): Append codepoint.


// Word array, can hold UCS2 or UTF-16.
// TODO(Matt): Add conversions to/from UTF8, UTF-32, etc.
// TODO(Matt): Add splice/remove, contains char/substring, etc.
// TODO(Matt): UTF-16 Length
struct Str16
{
    char16_t* ptr; // Buffer pointer. Allocated with USTRING_MALLOC, USTRING_REALLOC, and USTRING_FREE.
    int size; // Number of words used by the string, *excluding* the null terminator.
    int capacity; // Number of bytes reserved by the string, must be greater than size.
    
    // Default/list construction, construction from word buffer, and copy/move construction.
    Str16() = default;
	Str16(const char* str);
    Str16(const char16_t* str);
    Str16(const Str16& str);
    Str16(Str16&& str);
    // TODO(Matt): Construct from UTF8.
    // TODO(Matt): Construct from Codepoint buffer.
    // TODO(Matt): Construct with initial reserve.
    // TODO(Matt): Construct from pointer and count.
    ~Str16();
    
    // Copy-assignment and move-assignment.
    // TODO(Matt): Assign from word buffer.
    // TODO(Matt): Assign from UTF8 string.
    // TODO(Matt): Assign from Codepoint buffer.
    inline Str16& operator=(const Str16& str);
    inline Str16& operator=(Str16&& str);
    
    inline operator char16_t*() const {return ptr;} // Strings can auto-cast to char16_t*.
	inline operator wchar_t*() const {return (wchar_t*)ptr;} // TODO(Matt): Is this safe?
    // Array index operators.
    inline char16_t& operator[](int i) {return ptr[i];}
    inline const char16_t& operator[](int i) const {return ptr[i];}
    
    // Comparison operators.
    inline bool operator==(const Str16& str);
    inline bool operator==(const char16_t* str);
    
    // Append string, word buffer, or a single character.
    inline Str16& operator+=(const Str16& str);
    inline Str16& operator+=(const char16_t* str);
    inline Str16& operator+=(char16_t c);
    // TODO(Matt): Append UTF8 string.
    // TODO(Matt): Append Codepoint buffer.
    // TODO(Matt): Append UTF8 character.
    // TODO(Matt): Append codepoint.
    // TODO(Matt): Set reserve size.
};

// More append combinations.
inline Str16 operator+(const Str16& a, const Str16& b);
inline Str16 operator+(const Str16& a, const char16_t* b);
inline Str16 operator+(const char16_t* a, const Str16& b);
inline Str16 operator+(const Str16& a, char16_t b);
inline Str16 operator+(char16_t a, const Str16& b);
// TODO(Matt): Append UTF-8 string.
// TODO(Matt): Append Codepoint buffer.
// TODO(Matt): Append UTF-8 character.
// TODO(Matt): Append codepoint.

// Returns the new length
inline u32 StripWhitespace(char *str, u32 len)
{
    for (u32 i = 0; i < len; ++i)
    {
        if (str[i] == ' ')
        {
            for (u32 j = i; j < len-1; ++j)
            {
                str[j] = str[j+1];
            }
            
            len--;
        }
    }
    str[len] = 0;
    return len;
}

inline void StripWhitespace(Str8 *str)
{
    for (i32 i = 0; i < str->size; ++i)
    {
        if (str->ptr[i] == ' ')
        {
            // String is always null-terminated, so index = size
            // is the null terminator
            for (i32 j = i; j < str->size; ++j)
            {
                str->ptr[j] = str->ptr[j+1];
            }
            
            str->size--;
        }
    }
}

#endif

#ifdef USTRING_IMPLEMENTATION

#if !defined USTRING_MALLOC || !defined USTRING_REALLOC || !defined USTRING_FREE
#include <cstdlib>
#endif

#ifndef USTRING_ASSERT
#include <cassert>
#define USTRING_ASSERT assert
#endif

#ifndef USTRING_MALLOC
#define USTRING_MALLOC malloc
#endif

#ifndef USTRING_REALLOC
#define USTRING_REALLOC realloc
#endif

#ifndef USTRING_FREE
#define USTRING_FREE(x) { free(x); x = 0; }
#endif

#ifndef USTRING_MIN_RESERVE
#define USTRING_MIN_RESERVE 16
#endif

#ifndef USTRING_REPLACEMENT_CHAR
#define USTRING_REPLACEMENT_CHAR 0xfffd
#endif

int SizeInUTF8(char32_t cp)
{
    if (cp <= 0x7f) return 1;
    if (cp <= 0x7ff) return 2;
#ifdef USTRING_NO_UNPAIRED_SURROGATES
    if (cp >= 0xd800 && cp <= 0xdfff) return 0;
#endif
    if (cp <= 0xfffd) return 3;
    if (cp <= 0xffff) return 0; // 0xfffe and 0xffff are invalid.
    if (cp <= 0x10ffff) return 4;
    return 0; // Anything above 0x10ffff is invalid.
}

int SizeInUTF16(char32_t cp)
{
    if (cp <= 0xd7ff) return 1; // Low range of single-word codes.
    
    // If unpaired surrogates are disallowed, the range 0xd800..0xdfff is
    // invalid. Otherwise, we treat as a single word code.
#ifdef USTRING_NO_UNPAIRED_SURROGATES
    if (cp <= 0xdfff) return 0;
#endif
    
    if (cp <= 0xfffd) return 1; // High range of single-word codes.
    if (cp <= 0xffff) return 0; // 0xfffe and 0xffff are invalid.
    if (cp <= 0x10ffff) return 2; // Anything above 0xffff needs two words.
    return 0; // Anything above 0x10ffff is invalid.
}

int SizeFromLeading(char c)
{
    if ((c & 0x80) == 0x00) return 1; // All bytes in range 0..127 are valid.
    if (c == 0xc0 || c == 0xc1 || c > 0xf4) return 0; // Check illegal bytes.
    if ((c & 0xe0) == 0xc0) return 2;
    if ((c & 0xf0) == 0xe0) return 3;
    if ((c & 0xf8) == 0xf0) return 4;
    return 0; // Unrecognized leading byte.
}

int SizeFromLeading(char16_t c)
{
    if (c <= 0xd7ff) return 1; // Low range of single-word codes.
    if (c <= 0xdbff) return 2; // High surrogate of double-word codes.
    
    // If unpaired surrogates are disallowed, a trailing surrogate cannot
    // appear as the leading word. Otherwise just treat as a single word.
#ifdef USTRING_NO_UNPAIRED_SURROGATES
    if (c <= 0xdfff) return 0;
#endif
    if (c <= 0xfffd) return 1; // High range of single-word surrogates.
    return 0; // Code points 0xfffe and 0xffff are invalid.
}

char32_t ToCodePoint(const char utf8[4], int* consumed)
{
    *consumed = SizeFromLeading(utf8[0]);
    switch(*consumed)
    {
        case 1: return utf8[0] & 0x7f;
        case 2:
        {
            if ((utf8[1] & 0xc0) != 0x80) break;
            return ((utf8[0] & 0x1f) << 6) | (utf8[1] & 0x3f);
        }
        case 3:
        {
            if ((utf8[1] & 0xc0) != 0x80 || (utf8[2] & 0xc0) != 0x80) break;
            char32_t out = ((utf8[0] & 0x0f) << 12) |
                ((utf8[1] & 0x3f) << 6) |
                (utf8[2] & 0x3f);
            if (utf8[0] == 0xe0 && out < 0x0800) break;
            if (utf8[0] == 0xe4 && out > 0xd800 && out < 0xdfff) break;
            return out;
        }
        case 4:
        {
            if ((utf8[1] & 0xc0) != 0x80 ||
                (utf8[2] & 0xc0) != 0x80 ||
                (utf8[3] & 0xc0) != 0x80) break;
            char32_t out = ((utf8[0] & 0x07) << 18) |
                ((utf8[1] & 0x3f) << 12) |
                ((utf8[2] & 0x3f) << 6) |
                (utf8[3] & 0x3f);
            if (utf8[0] == 0xf0 && out < 0x10000) break;
            if (utf8[0] == 0xf4 && out > 0x10ffff) break;
            return out;
        }
    }
    *consumed = 1;
    return USTRING_REPLACEMENT_CHAR;
}

char32_t ToCodePoint(const char16_t utf16[2], int* consumed)
{
    *consumed = SizeFromLeading(utf16[0]);
    switch(*consumed)
    {
        case 1: return utf16[0];
        case 2:
        {
#ifdef USTRING_NO_UNPAIRED_SURROGATES
            if (utf16[1] < 0xdc00 || utf16[1] > 0xdfff) break;
#endif
            return ((utf16[0] - 0xd800) << 10) + (utf16[1] - 0xdc00) + 0x10000;
        }
    }
    *consumed = 1;
    return USTRING_REPLACEMENT_CHAR;
}

bool ToUTF8(char32_t cp, char out[4], int* size)
{
    *size = SizeInUTF8(cp);
    switch(*size)
    {
        case 1:
        {
            out[0] = (char)(cp);
            return true;
        }
        case 2:
        {
            out[0] = 0xc0 | ((cp >> 6) & 0x1f);
            out[1] = 0x80 | (cp & 0x3f);
            return true;
        }
        case 3:
        {
            out[0] = 0xe0 | ((cp >> 12) & 0x0f);
            out[1] = 0x80 | ((cp >> 6) & 0x3f);
            out[2] = 0x80 | (cp & 0x3f);
            return true;
        }
        case 4:
        {
            out[0] = 0xf0 | ((cp >> 18) & 0x07);
            out[1] = 0x80 | ((cp >> 12) & 0x3f);
            out[2] = 0x80 | ((cp >> 6) & 0x3f);
            out[3] = 0x80 | (cp & 0x3f);
            return true;
        }
        default:
        {
            *size = 3;
            out[0] = (unsigned char)(0xef);
            out[1] = (unsigned char)(0xbf);
            out[2] = (unsigned char)(0xbd);
            return false;
        }
    }
}

bool ToUTF16(char32_t cp, char16_t out[2], int* size)
{
    *size = SizeInUTF16(cp);
    switch(*size)
    {
        case 1:
        {
            out[0] = (char16_t)(cp);
            return true;
        }
        case 2:
        {
            cp -= 0x10000;
            out[0] = (char16_t)((cp >> 10) + 0xd800);
            out[1] = (cp & 0x3ff) + 0xdc00;
            return true;
        }
        default:
        {
            *size = 1;
            out[0] = 0xfffd;
            return false;
        }
    }
}

Str8::Str8(int reserve) : ptr(0), size(0), capacity(reserve + 1)
{
    ptr = (char*)USTRING_MALLOC(capacity);
    *ptr = 0;
}

#include <stdio.h>
Str8::Str8(const char* str) : ptr(0), size(0), capacity(0)
{
    if (!str || !str[0]) return;
    while (str[size]) ++size;
    capacity = (size < USTRING_MIN_RESERVE) ? USTRING_MIN_RESERVE : size + 1;
    ptr = (char*)USTRING_MALLOC(capacity);
    for (int i = 0; i <= size; ++i) ptr[i] = str[i];
}

Str8::Str8(const char* str, int length) : ptr(0), size(length), capacity(0)
{
    if (!str || !str[0]) return;
    capacity = (size < USTRING_MIN_RESERVE) ? USTRING_MIN_RESERVE : size + 1;
    ptr = (char*)USTRING_MALLOC(capacity);
    for (int i = 0; i < size; ++i) ptr[i] = str[i];
    ptr[size] = 0;
}

Str8::Str8(const char16_t* str) : ptr(0), size(0), capacity(0)
{
    if (!str || !str[0]) return;
    int i = 0;
    while (str[i])
    {
        int consumed;
        char32_t cp = ToCodePoint(&str[i], &consumed);
        i += consumed;
        size += SizeInUTF8(cp);
    }
    capacity = (USTRING_MIN_RESERVE < size) ? size + 1 : USTRING_MIN_RESERVE;
    ptr = (char*)USTRING_MALLOC(capacity);
    
    i = 0;
    int j = 0;
    while (str[i])
    {
        int consumed;
        char32_t cp = ToCodePoint(&str[i], &consumed);
        i += consumed;
        if (size < j) break;
        ToUTF8(cp, &ptr[j], &consumed);
        j += consumed;
    }
    ptr[size] = 0;
}

Str8::Str8(const char16_t* str, int length) : ptr(0), size(0), capacity(0)
{
    if (!str || !str[0]) return;
    int i = 0;
    while (i < length)
    {
        int consumed;
        char32_t cp = ToCodePoint(&str[i], &consumed);
        i += consumed;
        size += SizeInUTF8(cp);
    }
    capacity = (USTRING_MIN_RESERVE < size) ? size + 1 : USTRING_MIN_RESERVE;
    ptr = (char*)USTRING_MALLOC(capacity);
    
    i = 0;
    int j = 0;
    while (i < length)
    {
        int consumed;
        char32_t cp = ToCodePoint(&str[i], &consumed);
        i += consumed;
        if (size < j) break;
        ToUTF8(cp, &ptr[j], &consumed);
        j += consumed;
    }
    ptr[size] = 0;
}

Str8::Str8(const Str16& str) : Str8(str.ptr) {}

Str8::Str8(const Str8& str)
{
    size = str.size;
    capacity = str.capacity;
    ptr = (capacity) ? (char*)USTRING_MALLOC(capacity) : 0;
    for (int i = 0; i <= size; ++i) ptr[i] = str.ptr[i];
}

Str8::Str8(Str8&& str)
{
    size = str.size;
    capacity = str.capacity;
    ptr = str.ptr;
    str.ptr = 0;
    str.size = 0;
    str.capacity = 0;
}

Str8::Str8(const char32_t* str) : ptr(0), size(0), capacity(0)
{
    if (!str || !str[0]) return;
    for (int i = 0; str[i]; size += SizeInUTF8(str[i]));
    capacity = (USTRING_MIN_RESERVE < size) ? size + 1 : USTRING_MIN_RESERVE;
    ptr = (char*)USTRING_MALLOC(capacity);
    for (int i = 0, j = 0, consumed; str[size]; ++i)
    {
        if (size < j) break;
        ToUTF8(str[i], &ptr[j], &consumed);
        j += consumed;
    }
    ptr[size] = 0;
}

Str8::Str8(const char32_t* str, int length) : ptr(0), size(0), capacity(0)
{
    if (!str || !str[0]) return;
    for (int i = 0; i < length; ++i) size += SizeInUTF8(str[i]);
    capacity = (USTRING_MIN_RESERVE < size) ? size + 1 : USTRING_MIN_RESERVE;
    ptr = (char*)USTRING_MALLOC(capacity);
    for (int i = 0, j = 0, consumed; i < length; ++i)
    {
        if (size < j) break;
        ToUTF8(str[i], &ptr[j], &consumed);
        j += consumed;
    }
    ptr[size] = 0;
}

Str8::~Str8()
{
    if (ptr) USTRING_FREE(ptr);
    ptr = 0;
    size = 0;
    capacity = 0;
}

inline Str8& Str8::operator=(const Str8& str)
{
    if (&str == this || !str.ptr) return *this;
    if (ptr) USTRING_FREE(ptr);
    size = str.size;
    capacity = str.capacity;
    ptr = (char*)USTRING_MALLOC(capacity);
    for (int i = 0; i <= size; ++i) ptr[i] = str[i];
    return *this;
}

inline Str8 operator+(const Str8& a, const Str8& b)
{
    if (!a.ptr && !b.ptr) return {};
    if (!a.ptr) return b;
    if (!b.ptr) return a;
    Str8 result;
    result.size = a.size + b.size;
    result.capacity = (result.size < USTRING_MIN_RESERVE) ? USTRING_MIN_RESERVE : result.size + 1;
    result.ptr = (char*)USTRING_MALLOC(result.capacity);
    for (int i = 0; i < a.size; ++i) result.ptr[i] = a.ptr[i];
    for (int i = 0; i <= b.size; ++i) result.ptr[a.size + i] = b.ptr[i];
    return result;
}

inline Str8 operator+(const Str8& a, const char* b)
{
    if (!a.ptr && !b) return {};
    if (!a.ptr) return Str8(b);
    if (!b) return a;
    int b_size = 0;
    while (b[b_size]) ++b_size;
    Str8 result;
    result.size = a.size + b_size;
    result.capacity = (result.size < USTRING_MIN_RESERVE) ? USTRING_MIN_RESERVE : result.size + 1;
    result.ptr = (char*)USTRING_MALLOC(result.capacity);
    for (int i = 0; i < a.size; ++i) result.ptr[i] = a.ptr[i];
    for (int i = 0; i <= b_size; ++i) result.ptr[a.size + i] = b[i];
    return result;
}

inline Str8 operator+(const char* a, const Str8& b)
{
    if (!a && !b.ptr) return {};
    if (!a) return b;
    if (!b.ptr) return Str8(a);
    int a_size = 0;
    while (a[a_size]) ++a_size;
    Str8 result;
    result.size = a_size + b.size;
    result.capacity = (result.size < USTRING_MIN_RESERVE) ? USTRING_MIN_RESERVE : result.size + 1;
    result.ptr = (char*)USTRING_MALLOC(result.capacity);
    for (int i = 0; i < a_size; ++i) result.ptr[i] = a[i];
    for (int i = 0; i <= b.size; ++i) result.ptr[b.size + i] = b.ptr[i];
    return result;
}

inline Str8 operator+(const Str8& a, char b)
{
    if (!a.ptr && !b) return {};
    if (!a.ptr)
    {
        char str[2] = {b, 0};
        return Str8(str);
    }
    if (!b) return a;
    Str8 result;
    result.size = a.size + 1;
    result.capacity = (result.size < USTRING_MIN_RESERVE) ? USTRING_MIN_RESERVE : result.size + 1;
    result.ptr = (char*)USTRING_MALLOC(result.capacity);
    for (int i = 0; i < a.size; ++i) result.ptr[i] = a[i];
    result.ptr[a.size] = b;
    result.ptr[result.size] = 0;
    return result;
}

inline Str8 operator+(char a, const Str8& b)
{
    if (!a && !b.ptr) return {};
    if (!a) return b;
    if (!b.ptr)
    {
        char str[2] = {a, 0};
        return Str8(str);
    }
    Str8 result;
    result.size = 1 + b.size;
    result.capacity = (result.size < USTRING_MIN_RESERVE) ? USTRING_MIN_RESERVE : result.size + 1;
    result.ptr = (char*)USTRING_MALLOC(result.capacity);
    result.ptr[0] = a;
    for (int i = 0; i <= b.size; ++i) result.ptr[i + 1] = b.ptr[i];
    return result;
}

inline bool Str8::operator==(const Str8& str)
{
    if (this == &str || ptr == str.ptr) return true;
    if ((ptr != 0) != (str.ptr != 0) || size != str.size) return false;
    for (int i = 0; i < size; ++i) if (ptr[i] != str.ptr[i]) return false;
    return true;
}

inline Str8& Str8::operator=(Str8&& str)
{
    if (this == &str) return *this;
    if (ptr) USTRING_FREE(ptr);
    size = str.size;
    capacity = str.capacity;
    ptr = str.ptr;
    str.ptr = 0;
    str.size = 0;
    str.capacity = 0;
    return *this;
}

inline bool Str8::operator==(const char* str)
{
    if (ptr == str) return true;
    if ((ptr != 0) != (str != 0)) return false;
    for (int i = 0; i <= size; ++i) if (ptr[i] != str[i]) return false;
}

inline Str8& Str8::operator+=(const Str8& str)
{
    if (!str.ptr) return *this;
    if (!ptr) return *this = str;
    int base = size;
    size += str.size;
    if (size >= capacity)
    {
        capacity = (capacity * 2 > size) ? capacity * 2 : size;
        ptr = (char*)USTRING_REALLOC(ptr, capacity);
    }
    for (int i = 0; i <= str.size; ++i) ptr[base + i] = str[i];
    return *this;
}

inline Str8& Str8::operator+=(const char* str)
{
    if (!str) return *this;
    if (!ptr) return *this = str;
    int str_size = 0;
    while (str[str_size]) ++str_size;
    int base = size;
    size += str_size;
    if (size >= capacity)
    {
        capacity = (capacity * 2 > size) ? capacity * 2 : size;
        ptr = (char*)USTRING_REALLOC(ptr, capacity);
    }
    for (int i = 0; i <= str_size; ++i) ptr[base + i] = str[i];
    return *this;
}

inline Str8& Str8::operator+=(char c)
{
    if (!c) return *this;
    if (!ptr)
    {
        char str [2] = {c, 0};
        return *this = str;
    }
    if (size == capacity - 1)
    {
        capacity *= 2;
        ptr = (char*)USTRING_REALLOC(ptr, capacity);
    }
    ptr[size++] = c;
    ptr[size] = 0;
    return *this;
}

//////////////////////////////// Str16 /////////////////////////////////////////

Str16::Str16(const char16_t* str) : ptr(0), size(0), capacity(0)
{
    if (!str || !str[0]) return;
    while (str[size]) ++size;
    capacity = (USTRING_MIN_RESERVE < size) ? size + 1 : USTRING_MIN_RESERVE;
    ptr = (char16_t*)USTRING_MALLOC(capacity * 2);
    for (int i = 0; i <= size; ++i) ptr[i] = str[i];
}

Str16::Str16(const char* str) : ptr(0), size(0), capacity(0)
{
    if (!str || !str[0]) return;
    int i = 0;
    while (str[i])
    {
        int consumed;
        char32_t cp = ToCodePoint(&str[i], &consumed);
        i += consumed;
        size += SizeInUTF16(cp);
    }
    capacity = (USTRING_MIN_RESERVE < size) ? size + 1 : USTRING_MIN_RESERVE;
    ptr = (char16_t*)USTRING_MALLOC(capacity * 2);
    
    i = 0;
    int j = 0;
    while (str[i])
    {
        int consumed;
        char32_t cp = ToCodePoint(&str[i], &consumed);
        i += consumed;
        if (size < j) break;
        ToUTF16(cp, &ptr[j], &consumed);
        j += consumed;
    }
    ptr[size] = 0;
}

Str16::Str16(const Str16& str)
{
    size = str.size;
    capacity = str.capacity;
    ptr = (capacity) ? (char16_t*)USTRING_MALLOC(capacity * 2) : 0;
    for (int i = 0; i <= size; ++i) ptr[i] = str.ptr[i];
}

Str16::Str16(Str16&& str)
{
    size = str.size;
    capacity = str.capacity;
    ptr = str.ptr;
    str.ptr = 0;
    str.size = 0;
    str.capacity = 0;
}

inline Str16& Str16::operator=(Str16&& str)
{
    if (this == &str) return *this;
    if (ptr) USTRING_FREE(ptr);
    size = str.size;
    capacity = str.capacity;
    ptr = str.ptr;
    str.ptr = 0;
    str.size = 0;
    str.capacity = 0;
    return *this;
}

Str16::~Str16()
{
    if (ptr) USTRING_FREE(ptr);
    size = 0;
}

inline Str16& Str16::operator=(const Str16& str)
{
    if (&str == this || !str.ptr) return *this;
    if (ptr) USTRING_FREE(ptr);
    size = str.size;
    capacity = str.capacity;
    ptr = (char16_t*)USTRING_MALLOC(capacity * 2);
    for (int i = 0; i <= size; ++i) ptr[i] = str[i];
    return *this;
}

inline Str16 operator+(const Str16& a, const Str16& b)
{
    if (!a.ptr && !b.ptr) return {};
    if (!a.ptr) return b;
    if (!b.ptr) return a;
    Str16 result;
    result.size = a.size + b.size;
    result.capacity = (result.size < USTRING_MIN_RESERVE) ? USTRING_MIN_RESERVE : result.size + 1;
    result.ptr = (char16_t*)USTRING_MALLOC(result.capacity * 2);
    for (int i = 0; i < a.size; ++i) result.ptr[i] = a.ptr[i];
    for (int i = 0; i <= b.size; ++i) result.ptr[a.size + i] = b.ptr[i];
    return result;
}

inline Str16 operator+(const Str16& a, const char16_t* b)
{
    if (!a.ptr && !b) return {};
    if (!a.ptr) return Str16(b);
    if (!b) return a;
    int b_size = 0;
    while (b[b_size]) ++b_size;
    Str16 result;
    result.size = a.size + b_size;
    result.capacity = (result.size < USTRING_MIN_RESERVE) ? USTRING_MIN_RESERVE : result.size + 1;
    result.ptr = (char16_t*)USTRING_MALLOC(result.capacity * 2);
    for (int i = 0; i < a.size; ++i) result.ptr[i] = a.ptr[i];
    for (int i = 0; i <= b_size; ++i) result.ptr[a.size + i] = b[i];
    return result;
}

inline Str16 operator+(const char16_t* a, const Str16& b)
{
    if (!a && !b.ptr) return {};
    if (!a) return b;
    if (!b.ptr) return Str16(a);
    int a_size = 0;
    while (a[a_size]) ++a_size;
    Str16 result;
    result.size = a_size + b.size;
    result.capacity = (result.size < USTRING_MIN_RESERVE) ? USTRING_MIN_RESERVE : result.size + 1;
    result.ptr = (char16_t*)USTRING_MALLOC(result.capacity * 2);
    for (int i = 0; i < a_size; ++i) result.ptr[i] = a[i];
    for (int i = 0; i <= b.size; ++i) result.ptr[b.size + i] = b.ptr[i];
    return result;
}

inline Str16 operator+(const Str16& a, char16_t b)
{
    if (!a.ptr && !b) return {};
    if (!a.ptr)
    {
        char16_t str[2] = {b, 0};
        return Str16(str);
    }
    if (!b) return a;
    Str16 result;
    result.size = a.size + 1;
    result.capacity = (result.size < USTRING_MIN_RESERVE) ? USTRING_MIN_RESERVE : result.size + 1;
    result.ptr = (char16_t*)USTRING_MALLOC(result.capacity * 2);
    for (int i = 0; i < a.size; ++i) result.ptr[i] = a[i];
    result.ptr[a.size] = b;
    result.ptr[result.size] = 0;
    return result;
}

inline Str16 operator+(char16_t a, const Str16& b)
{
    if (!a && !b.ptr) return {};
    if (!a) return b;
    if (!b.ptr)
    {
        char16_t str[2] = {a, 0};
        return Str16(str);
    }
    Str16 result;
    result.size = 1 + b.size;
    result.capacity = (result.size < USTRING_MIN_RESERVE) ? USTRING_MIN_RESERVE : result.size + 1;
    result.ptr = (char16_t*)USTRING_MALLOC(result.capacity * 2);
    result.ptr[0] = a;
    for (int i = 0; i <= b.size; ++i) result.ptr[i + 1] = b.ptr[i];
    return result;
}

inline bool Str16::operator==(const Str16& str)
{
    if (this == &str || ptr == str.ptr) return true;
    if ((ptr != 0) != (str.ptr != 0) || size != str.size) return false;
    for (int i = 0; i < size; ++i) if (ptr[i] != str.ptr[i]) return false;
    return true;
}

inline bool Str16::operator==(const char16_t* str)
{
    if (ptr == str) return true;
    if ((ptr != 0) != (str != 0)) return false;
    for (int i = 0; i <= size; ++i) if (ptr[i] != str[i]) return false;
}

inline Str16& Str16::operator+=(const Str16& str)
{
    if (!str.ptr) return *this;
    if (!ptr) return *this = str;
    int base = size;
    size += str.size;
    if (size >= capacity)
    {
        capacity = (capacity * 2 > size) ? capacity * 2 : size;
        ptr = (char16_t*)USTRING_REALLOC(ptr, capacity * 2);
    }
    for (int i = 0; i <= str.size; ++i) ptr[base + i] = str[i];
    return *this;
}

inline Str16& Str16::operator+=(const char16_t* str)
{
    if (!str) return *this;
    if (!ptr) return *this = str;
    int str_size = 0;
    while (str[str_size]) ++str_size;
    int base = size;
    size += str_size;
    if (size >= capacity)
    {
        capacity = (capacity * 2 > size) ? capacity * 2 : size;
        ptr = (char16_t*)USTRING_REALLOC(ptr, capacity * 2);
    }
    for (int i = 0; i <= str_size; ++i) ptr[base + i] = str[i];
    return *this;
}

inline Str16& Str16::operator+=(char16_t c)
{
    if (!c) return *this;
    if (!ptr)
    {
        char16_t str [2] = {c, 0};
        return *this = str;
    }
    if (size == capacity - 1)
    {
        capacity *= 2;
        ptr = (char16_t*)USTRING_REALLOC(ptr, capacity * 2);
    }
    ptr[size++] = c;
    ptr[size] = 0;
    return *this;
}

#endif