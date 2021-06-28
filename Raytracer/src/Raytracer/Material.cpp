
file_internal bool 
material_scatter(HitRecord *record, Ray *ray, Ray *scattered_ray, v3 *attentuation)
{
    bool Result = false;
    
    switch (record->material.type)
    {
        case Material_Lambertian:
        {
            v3 ScatterDir = v3_add(record->normal, random_unit_vector());
            if (v3_near_zero(ScatterDir))
            {
                ScatterDir = record->normal;
            }
            
            scattered_ray->orig = record->point;
            scattered_ray->dir = ScatterDir;
            scattered_ray->time = ray->time;
            *attentuation = record->material.lambertian.albedo;
            Result = true;
        } break;
        
        case Material_Metal:
        {
            v3 Reflected = reflect(v3_norm(ray->dir), record->normal);
            
            scattered_ray->orig = record->point;
            scattered_ray->dir    = v3_add(Reflected, v3_mulf(random_in_unit_sphere(), record->material.metal.fuzz));
            scattered_ray->time = ray->time;
            *attentuation = record->material.metal.albedo;
            Result = (v3_dot(scattered_ray->dir, record->normal) > 0.0f);
        } break;
        
        case Material_Dielectric:
        {
            *attentuation = V3_ONE;
            
            r32 Ratio = (record->is_front_facing) ? 1.0f / record->material.dielectric.ior : record->material.dielectric.ior;
            
            v3 UnitDir = v3_norm(ray->dir);
            r32 CosTheta = fminf(v3_dot(v3_mulf(UnitDir, -1.0f), record->normal), 1.0f);
            r32 SinTheta = sqrtf(1.0f - CosTheta * CosTheta);
            
            if (Ratio * SinTheta > 1.0f)
            {
                v3 Reflected = reflect(UnitDir, record->normal);
                scattered_ray->orig = record->point;
                scattered_ray->dir = Reflected;
                scattered_ray->time = ray->time;
            }
            else
            {
                r32 ReflectProb = schlick(CosTheta, Ratio);
                if (random() < ReflectProb)
                {
                    v3 Reflected = reflect(UnitDir, record->normal);
                    scattered_ray->orig = record->point;
                    scattered_ray->dir = Reflected;
                    scattered_ray->time = ray->time;
                }
                else
                {
                    scattered_ray->orig = record->point;
                    scattered_ray->dir = refract(UnitDir, record->normal, Ratio);
                    scattered_ray->time = ray->time;
                }
            }
            
            Result = true;
        };
    }
    
    return Result;
}

file_internal void 
make_lambertian(Material *mat, v3 Albedo)
{
    mat->type = Material_Lambertian;
    mat->lambertian.albedo = Albedo;
}

file_internal void
make_metal(Material *mat, v3 Albedo, r32 Fuzz)
{
    mat->type = Material_Metal;
    mat->metal.albedo = Albedo;
    mat->metal.fuzz = Fuzz;
}

file_internal void 
make_dielectric(Material *mat, r32 IoR)
{
    mat->type = Material_Dielectric;
    mat->dielectric.ior = IoR;
}