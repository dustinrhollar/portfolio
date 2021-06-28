#ifndef _RAYTRACER_CAMERA_H
#define _RAYTRACER_CAMERA_H

struct CameraCreateInfo
{
    v3  look_from; 
    v3  look_at; 
    v3  up;
    r32 vfov; 
    r32 aspect_ratio; 
    r32 aperture; 
    r32 focus_dist;
    r32 t0;
    r32 t1;
};

struct Camera
{
    v3 origin;
    v3 lower_left_corner;
    v3 horizontal;
    v3 vertical;
    
    v3 u, v, w;
    r32 lens_radius;
    r32 time0, time1; // camera shutter open/close times
};

file_internal void camera_init(Camera *camera, CameraCreateInfo *info);
file_internal void camera_get_ray(struct Ray *ray, Camera *camera, r32 s, r32 t);

#endif //_RAYTRACER_CAMERA_H
