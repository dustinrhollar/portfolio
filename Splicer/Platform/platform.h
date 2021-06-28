#ifndef PLATFORM_H
#define PLATFORM_H

#include <vulkan/vulkan.h>
#include <jackal_types.h>

// TODO(Dustin): Replace vector with DynamicArray
//#include <dynamic_array.h>
#include <vector>

enum KeyInput
{
    KEY_PRESS_W = 1<<0,
    KEY_PRESS_S = 1<<1,
    KEY_PRESS_A = 1<<2,
    KEY_PRESS_D = 1<<3,
    KEY_PRESS_1 = 1<<4,
    KEY_PRESS_2 = 1<<5,
    KEY_PRESS_3 = 1<<6,
    KEY_PRESS_4 = 1<<7,
    KEY_PRESS_UP    = 1<<8,
    KEY_PRESS_DOWN  = 1<<9,
    KEY_PRESS_LEFT  = 1<<10,
    KEY_PRESS_RIGHT = 1<<11,
    
    KEY_PRESS_I = 1<<12,
    KEY_PRESS_O = 1<<13,
    KEY_PRESS_P = 1<<14,
};

struct GameState
{
    bool DevMode; // true = dev mode active
    
    float mouse_xpos;
    float mouse_ypos;
    
    float time;
    u32 input_bitfield; // 32 keys allowed for now
};

//#define PLATFORM_READ_FILE(name) void name(FileInfo *info, const char *filename)
//typedef PLATFORM_READ_FILE(platform_read_file);

#define PLATFORM_GET_WALL_CLOCK(name) u64 name()
typedef PLATFORM_GET_WALL_CLOCK(platform_get_wall_clock);

#define PLATFORM_GET_SECONDS_ELAPSED(name) float name(u64 start, u64 end)
typedef PLATFORM_GET_SECONDS_ELAPSED(platform_get_seconds_elapsed);

//#define PLATFORM_GET_WINDOW(name) SysWindow name()
//typedef PLATFORM_GET_WINDOW(platform_get_window);

#define PLATFORM_GET_CLIENT_WINDOW_DIMENSIONS(name) void name(int *width, int *height)
typedef PLATFORM_GET_CLIENT_WINDOW_DIMENSIONS(platform_get_client_window_dimensions);

#define PLATFORM_LOG(name) void name(char *fmt, ...)
typedef PLATFORM_LOG(platform_log);

// @param fullpath    pointer to a buffer the fullpath is copied into
// @param buffer_size size of the buffer
// @param filename    name of the file to get the full path for
// @param size        size of the filename
// @return the size of the string
#define PLATFORM_GET_FULL_FILEPATH(name) int name(char *fullpath, char *filename, size_t size)
typedef PLATFORM_GET_FULL_FILEPATH(platform_get_full_filepath);

//#define PLATFORM_GET_REQUIRED_INSTANCE_EXTENSIONS(name) std::vector<const char*> name(bool validation_layers_enabled)
#define PLATFORM_GET_REQUIRED_INSTANCE_EXTENSIONS(name) const char* name(bool validation_layers_enabled)
typedef PLATFORM_GET_REQUIRED_INSTANCE_EXTENSIONS(platform_get_required_instance_extensions);

// Params...
// u32 src_size
// char *src
// wchar_t *dst  -> pointer that is currently nullptr, data saved here
// u32 *dst_size -> pointer to store the resulting size
//
// It is the responsibility of the caller to free the
// the dst pointer.
#define PLATFORM_UTF8_TO_UTF16(name) void name(u32 src_size, char *src, u32 *dst_size, wchar_t **dst)
typedef PLATFORM_UTF8_TO_UTF16(platform_utf8_to_utf16);

#define PLATFORM_CREATE_SURFACE(name) void name(VkSurfaceKHR *surface, VkInstance instance)
typedef PLATFORM_CREATE_SURFACE(platform_create_surface);

#define PLATFORM_WAIT_FOR_EVENTS(name) void name()
typedef PLATFORM_WAIT_FOR_EVENTS(platform_wait_for_events);

typedef struct
{
    /*
    platform_read_file               *ReadFile;
    platform_get_full_filepath       *GetFullFilepath;
    platform_get_client_size         *GetClientSize;
    platform_get_window              *GetWindow;
    platform_log                     *Log;
    platform_utf8_to_utf16           *UTF8ToUTF16;
    */
    platform_get_wall_clock                   *GetWallClock;
    platform_get_seconds_elapsed              *GetSecondsElapsed;
    platform_get_required_instance_extensions *GetRequiredInstanceExtensions;
    platform_create_surface                   *CreateSurface;
    platform_get_client_window_dimensions     *GetClientWindowDimensions;
    platform_wait_for_events                  *WaitForEvents;
} platform_api;

// defined in:
// 1. splicer_win32.cpp <- entry point
// 2. splicer.cpp       <- dll entry point
extern platform_api Platform;

#define GAME_INITIALIZE(name) bool name(platform_api platform, void *glfw_proc_address)
typedef GAME_INITIALIZE(game_initialize);

#define GAME_UPDATE(name) void name(GameState game_state)
typedef GAME_UPDATE(game_update);

#define GAME_RESIZE(name) void name(u32 width, u32 height)
typedef GAME_RESIZE(game_resize);

#define GAME_SHUTDOWN(name) void name()
typedef GAME_SHUTDOWN(game_shutdown);

typedef struct
{
    game_initialize *Initialize;
    game_update     *Update;
    game_resize     *Resize;
    game_shutdown   *Shutdown;
    bool is_valid;
} GameApi;

#endif //PLATFORM_H
