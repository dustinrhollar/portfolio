#include <mm.h>

#define pstring_alloc(s)   palloc(s)
#define pstring_realloc(src, sz) prealloc((src), (sz))
#define pstring_free(p)    pfree(p)

#define tstring_alloc(s)   palloc(s)
#define tstring_realloc(s) prealloc(s)
#define tstring_free(p)    pfree(p)

//#define tstring_alloc(s) talloc(s)
//#define tstring_free(p)

#define USE_JENGINE_JSTRING_IMPLEMENTATION
#include "jstring.h"
