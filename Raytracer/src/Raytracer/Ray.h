#ifndef _RAYTRACER_RAY_H
#define _RAYTRACER_RAY_H

struct Ray
{
    v3  orig;
    v3  dir;
    r32 time;
};

file_internal v3 ray_move_along(Ray *ray, r32 t);
file_internal v3 ray_at(Ray *ray, r32 t);
file_internal v3 ray_color(Ray *ray, struct Scene *scene, u32 depth);

#endif //_RAYTRACER_RAY_H
