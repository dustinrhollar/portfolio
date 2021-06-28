
#if defined(__linux__) || defined(__APPLE__) 

#define __USE_GNU

#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <libgen.h>
#include <fcntl.h>
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>
#include <execinfo.h>
#include <unistd.h>
#include <signal.h>
#include <ucontext.h>

#include "X11/X11Logger.c"
#include "X11/X11Window.c"
#include "X11/X11CoreUtils.c"
#include "X11/X11File.c"
#include "X11/X11OpenGL.c"
#include "X11/X11Main.c"

#else
#error Platform Not Supported!
#endif
