#ifndef _SHADER_DATA_
#define _SHADER_DATA_

#define SHADER_UBO_BINDING_GLOBAL   0
#define SHADER_UBO_BINDING_LIGHTING 1

typedef struct
{
    v3 color;
    r32 pad0;
    v3 dir;
    r32 pad1;
    v2 bias;
} Light;

// Will the assignment: alignas(16) v3 field;
// work instead of adding padding?
typedef struct
{
    m4  proj;
    m4  view;
    v3  cam_pos;
    r32 pad0;
} ShaderGlobalData;

typedef struct
{
    v3  light_color;
    r32 pad0;
    v3  light_dir;
    r32 pad1;
    v2  light_bias;
    v2  pad3;
} ShaderLightingData;

#endif//_SHADER_DATA
