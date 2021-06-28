#ifndef _RAYTRACER_INTERSECTION_H
#define _RAYTRACER_INTERSECTION_H

struct HitRecord
{
    Material material;
    v3       point;
    v3       normal;
    b8       is_front_facing;
    r32      t;
};

struct Aabb
{
    v3 min;
    v3 max;
};

file_internal void hit_rec_set_face_normal(HitRecord *record, struct Ray *ray);

file_internal bool hit_sphere(v3 *center, r32 radius, struct Ray *r);

file_internal bool intersect_ray_scene(HitRecord    *record, 
                                       struct Ray   *ray, 
                                       struct Scene *scene,
                                       r32           tmin, 
                                       r32           tmax);
file_internal bool intersect_ray_primitive(HitRecord        *record,
                                           struct Ray       *ray,
                                           struct Primitive *primitive,
                                           r32               tmin, 
                                           r32              *tmax);
file_internal bool intersect_ray_dynamic_sphere(HitRecord                     *record,
                                                struct Ray                    *ray,
                                                struct PrimitiveDynamicSphere *sphere,
                                                r32                            tmin,
                                                r32                            tmax);
file_internal bool intersect_ray_sphere(HitRecord              *record, 
                                        struct Ray             *ray, 
                                        struct PrimitiveSphere *sphere, 
                                        r32                     tmin, 
                                        r32                     tmax);

file_internal bool intersect_ray_aabb(struct Ray *ray, Aabb *aabb, r32 tmin, r32 tmax);

file_internal void build_surrounding_box(Aabb *result, Aabb *box0, Aabb *box1);
file_internal void build_aabb_sphere(Aabb *aabb, struct PrimitiveSphere *sphere, r32 t0, r32 t1);
file_internal void build_aabb_dyn_sphere(Aabb *aabb, struct PrimitiveDynamicSphere *sphere, r32 t0, r32 t1);
file_internal void build_aabb_primitive(Aabb *aabb, struct Primitive *primitive, r32 t0, r32 t1);
// Builds an aabb surrounding all objects in the list
file_internal void build_aabb_list(Aabb *aabb, struct Primitive *primitives, u32 count, r32 t0, r32 t1);

#endif //_INTERSECTION_H
