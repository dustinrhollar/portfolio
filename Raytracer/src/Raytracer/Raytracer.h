#ifndef _RAYTRACER_H
#define _RAYTRACER_H

typedef struct 
{
    u32            width;
    u32            height;
    u32            samples;
    u32            depth;
    r32           *image;
    struct Scene  *scene;
    struct Camera *camera;
} RaytracerSettings;

struct RtJob
{
    volatile u32      *counter;
    RaytracerSettings *settings;
    i32                dims[2]; // x, y dimensions for job size
    i32                pos[2];  // x, y position into world
};

file_internal void rt_entry(RaytracerSettings *settings);
file_internal void rt_async(void *args);
file_internal void rt_to_ppm(r32 *image, u32 width, u32 height);

#endif //_RAYTRACER_H
