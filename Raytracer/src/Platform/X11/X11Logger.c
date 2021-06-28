
typedef struct {
    va_list     ap;
    const char* fmt;
    const char* file;
    struct tm*  time;
    void*       udata;
    int         line;
    int         level;
} LogEvent;

file_global const char *g_log_level_strings[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};


file_global const char *g_log_level_colors[] = {
    "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};

#if defined(__x86_64__)
#define _PC gregs[REG_RSP]
#define _SP gregs[REG_RIP]
#else
#error architecture not supported
#endif

static void log_seg_handler(int sig, siginfo_t *psi, void* ctxarg)
{
    void *trace[10];
    char **messages = (char**)0;
    int i, trace_size = 0;
    mcontext_t *ctxP = &((ucontext_t *) ctxarg)->uc_mcontext;
    
    trace_size = backtrace(trace, 10);
    /* overwrite sigaction with caller's address */
    trace[1] = (void*)ctxP->_SP;
    messages = backtrace_symbols(trace, trace_size);
    /* skip first stack frame (points here) */
    LogError("Seg Fault Exectuion Path: Signal %d, Family Addr: %p from %p", sig, (void*)ctxP->_PC, (void*)ctxP->_SP);
    for (i = 0; i < trace_size; ++i)
    {
        fprintf(stderr, "\t#%d %s :: ", i, messages[i]);
        
        /* find first occurence of '(' or ' ' in message[i] and assume
         * everything before that is the file name. (Don't go beyond 0 though
         * (string terminator)*/
        size_t p = 0;
        while(messages[i][p] != '(' && messages[i][p] != ' '
              && messages[i][p] != 0)
        {
            ++p;
        }
        
        char syscom[256];
        sprintf(syscom,"addr2line %p -e %.*s", trace[i], (int)p, messages[i]);
        //last parameter is the file name of the symbol
        system(syscom);
    }
    
    exit(1);
}

static void StdoutCallback(LogEvent *ev) {
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';
    fprintf(ev->udata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
            buf, g_log_level_colors[ev->level], g_log_level_strings[ev->level],
            ev->file, ev->line);
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}

static void InitEvent(LogEvent *ev, void *udata) {
    if (!ev->time) {
        time_t t = time(NULL);
        ev->time = localtime(&t);
    }
    ev->udata = udata;
}

void PlatformLog(int level, const char *file, int line, const char *fmt, ...)
{
    LogEvent ev = {
        .fmt   = fmt,
        .file  = file,
        .line  = line,
        .level = level,
    };
    
    InitEvent(&ev, stderr);
    va_start(ev.ap, fmt);
    StdoutCallback(&ev);
    va_end(ev.ap);
    
    if (level == LOG_FATAL) 
    {
        // TODO(Dustin): Show MessageDialogBox instead....
        exit(1);
    }
    
    // TODO(Dustin): 
    // - Custom Callbacks
    // - Custom FILEs
}

