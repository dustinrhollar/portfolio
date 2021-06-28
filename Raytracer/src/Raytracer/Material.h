#ifndef _RAYTRACER_MATERIAL_H
#define _RAYTRACER_MATERIAL_H

enum MaterialType
{
    Material_Lambertian,
    Material_Metal,
    Material_Dielectric,
};

struct MaterialLambertian 
{
    v3 albedo;
};

struct MaterialMetal
{
    v3 albedo;
    r32 fuzz;
};

struct MaterialDielectric
{
    r32 ior; // index of refraction
};

struct Material
{
    MaterialType type;
    union
    {
        MaterialLambertian lambertian;
        MaterialMetal      metal;
        MaterialDielectric dielectric;
    };
};

file_internal bool material_scatter(struct HitRecord *record, struct Ray *ray, struct Ray *scattered_ray, v3 *attentuation);

#endif //_RAYTRACER_MATERIAL_H
