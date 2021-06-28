
file_internal v3 
ray_move_along(Ray *ray, r32 t)
{
    v3 result;
    // orig + dir*t
    result = v3_add(ray->orig, v3_mulf(ray->dir, t));
    return result;
}

file_internal v3 
ray_at(Ray *ray, r32 t) 
{
    return v3_add(ray->orig, v3_mulf(ray->dir, t));
}

file_internal v3 
ray_color(Ray *ray, struct Scene *scene, u32 depth)
{
    v3 result = V3_ZERO;
    
    if (depth > 0)
    {
        HitRecord record{};
        if (intersect_ray_scene(&record, ray, scene, 0.001f, R32_MAX))
        {
            Ray scattered{};
            v3 attent;
            
            if (material_scatter(&record, ray, &scattered, &attent))
            {
                result = v3_mul(ray_color(&scattered, scene, depth - 1), attent);
            }
        }
        else
        {
            v3 unit_dir = v3_norm(ray->dir);
            r32 t = 0.5f * (unit_dir.y + 1.0f);
            
            v3 white = { 0.5f, 0.5f, 0.5f };
            v3 blue  = {  0.5f, 0.7f, 1.0f };
            
            result = v3_add(v3_mulf(white, (1.0f - t)), v3_mulf(blue, t));
        }
    }
    
    return result;
}
