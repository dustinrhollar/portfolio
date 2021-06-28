#ifndef _RAYTRACER_PRIMITIVE_H
#define _RAYTRACER_PRIMITIVE_H

enum PrimitiveType
{
    Primitive_Sphere        = 0,
    Primitive_DynamicSphere = 1,
    Primitive_BvhNode       = 2,
};

struct PrimitiveSphere 
{
    v3       origin;
    r32      radius;
    Material material;
};

struct PrimitiveDynamicSphere 
{
    r32      radius;
    Material material;
    r32      t0;
    r32      t1;
    v3       c0;
    v3       c1;
};

// TODO(Dustin): Proper memory handling  for BVH Node
struct BvhNode
{
    struct Primitive *left;
    struct Primitive *right;
    Aabb              aabb;
};

struct Primitive 
{
    PrimitiveType type;
    union
    {
        PrimitiveDynamicSphere dyn_sphere;
        PrimitiveSphere        sphere;
        BvhNode                bvh_node;
    };
};

FORCE_INLINE v3 get_sphere_center(PrimitiveDynamicSphere *sphere, r32 Time)
{
    return v3_add(sphere->c0, v3_mulf(v3_sub(sphere->c1, sphere->c0), ((Time - sphere->t0) / (sphere->t1 - sphere->t0))));
}

file_internal void make_sphere(Primitive *prim, v3 origin, r32 radius,  Material material);
file_internal void make_dynamic_sphere(Primitive *prim, v3 center0, v3 center1,
                                       r32 time0, r32 time1, r32 radius,
                                       Material material);

file_internal void make_bvh_node(Primitive *result, Primitive *primitives, u32 start, u32 end, r32 t0, r32 t1);

#endif //_RAYTRACER_PRIMITIVE_H
