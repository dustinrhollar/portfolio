
file_internal bool intersect_ray_sphere(HitRecord *record, Ray *ray, Primitive *prim, r32 tmin, r32 *tmax);
file_internal bool intersect_ray_dynamic_sphere(HitRecord *Record,
                                                Ray       *ray,
                                                Primitive *prim,
                                                r32        Tmin,
                                                r32        *tmax);
file_internal bool intersect_ray_bvh_node(HitRecord  *record,
                                          Ray        *ray,
                                          Primitive  *prim,
                                          r32         tmin, 
                                          r32        *tmax);

typedef bool (*intersection_pfn)(HitRecord *record, Ray *ray, Primitive *sphere, r32 tmin, r32 *tmax);
file_global intersection_pfn g_intersection_look_up[] = {
    intersect_ray_sphere,         // Sphere
    intersect_ray_dynamic_sphere, // Dynamic Sphere
    intersect_ray_bvh_node,       // BVH Node
};

file_internal void 
hit_rec_set_face_normal(HitRecord *record, struct Ray *ray)
{
    record->is_front_facing = v3_dot(ray->dir, record->normal) < 0.0f;
    if (!record->is_front_facing) record->normal = v3_mulf(record->normal, -1.0f);
}

file_internal bool 
hit_sphere(v3 *center, r32 radius, Ray *r)
{
    v3 oc = v3_sub(r->orig, *center);
    r32 a = v3_dot(r->dir, r->dir);
    r32 b = 2.0f * v3_dot(oc, r->dir);
    r32 c = v3_dot(oc, oc) - radius*radius;
    r32 discriminant = b*b - 4.0f * a * c;
    return (discriminant > 0);
}

file_internal bool 
intersect_ray_sphere(HitRecord *record, Ray *ray, Primitive *prim, r32 tmin, r32 *Tmax)
{
    PrimitiveSphere *sphere = &prim->sphere;
    r32 tmax = *Tmax;
    
    bool Result = false;
    
    v3 dir = v3_sub(ray->orig, sphere->origin);
    
    r32 A = v3_mag_sq(ray->dir);
    r32 B = v3_dot(dir, ray->dir);
    r32 C = v3_mag_sq(dir) - sphere->radius * sphere->radius;
    
    r32 discriminant = (B * B) - (A * C);
    if (discriminant > 0)
    {
        r32 root = sqrtf(discriminant);
        r32 tmp = (-B - root) / (A);
        
        if (tmp < tmax && tmp > tmin)
        {
            record->t = tmp;
            record->point = ray_move_along(ray, record->t);
            record->normal = v3_divf(v3_sub(record->point, sphere->origin), sphere->radius);
            record->material = sphere->material;
            hit_rec_set_face_normal(record, ray);
            Result = true;
        }
        else
        {
            tmp = (-B + root) / (A);
            if (tmp < tmax && tmp > tmin)
            {
                record->t = tmp;
                record->point = ray_move_along(ray, record->t);
                record->normal = v3_divf(v3_sub(record->point, sphere->origin), sphere->radius);
                record->material = sphere->material;
                hit_rec_set_face_normal(record, ray);
                Result = true;
            }
        }
    }
    
    return Result;
}

file_internal bool 
intersect_ray_dynamic_sphere(HitRecord               *Record,
                             Ray                     *ray,
                             Primitive *prim,
                             r32                      Tmin,
                             r32                      *tmax)
{
    PrimitiveDynamicSphere  *sphere = &prim->dyn_sphere;
    r32 Tmax = *tmax;
    
    bool Result = false;
    v3 SphereCenter = get_sphere_center(sphere, ray->time);
    
    v3 Dir = v3_sub(ray->orig, SphereCenter);
    
    r32 A = v3_mag_sq(ray->dir);
    r32 B = v3_dot(Dir, ray->dir);
    r32 C = v3_mag_sq(Dir) - sphere->radius * sphere->radius;
    
    r32 Discriminant = (B * B) - (A * C);
    if (Discriminant > 0)
    {
        r32 Root = sqrtf(Discriminant);
        r32 Tmp = (-B - Root) / (A);
        
        if (Tmp < Tmax && Tmp > Tmin)
        {
            Record->t = Tmp;
            Record->point = ray_move_along(ray, Record->t);
            Record->normal = v3_divf(v3_sub(Record->point, SphereCenter), sphere->radius);
            Record->material = sphere->material;
            hit_rec_set_face_normal(Record, ray);
            Result = true;
        }
        else
        {
            Tmp = (-B + Root) / (A);
            if (Tmp < Tmax && Tmp > Tmin)
            {
                Record->t = Tmp;
                Record->point = ray_move_along(ray, Record->t);
                Record->normal = v3_divf(v3_sub(Record->point, SphereCenter), sphere->radius);
                Record->material = sphere->material;
                hit_rec_set_face_normal(Record, ray);
                Result = true;
            }
        }
    }
    
    return Result;
}

file_internal bool 
intersect_ray_aabb(Ray *ray, Aabb *aabb, r32 tmin, r32 tmax)
{
    for (int a = 0; a < 3; ++a)
    {
        // Fast AABB
        r32 invd = 1.0f / ray->dir.p[a];
        r32 t0 = (aabb->min.p[a] - ray->orig.p[a]) * invd;
        r32 t1 = (aabb->max.p[a] - ray->orig.p[a]) * invd;
        if (t1 < t0)
        {
            fast_swapf(t0, t1);
        }
        
        tmin = t0 > tmin ? t0 : tmin;
        tmax = t1 < tmax ? t1 : tmax;
        if (tmax < tmin) return false;
    }
    return true;
}

file_internal bool 
intersect_ray_bvh_node(HitRecord  *record,
                       Ray        *ray,
                       Primitive *prim,
                       r32         tmin, 
                       r32        *tmax)
{
    BvhNode *bvh_node = &prim->bvh_node;
    
    if (!intersect_ray_aabb(ray, &bvh_node->aabb, tmin, *tmax))
    {
        return false;
    }
    
    bool hit_left  = intersect_ray_primitive(record, ray, bvh_node->left,  tmin, tmax);
    bool hit_right = intersect_ray_primitive(record, ray, bvh_node->right, tmin, hit_left ? &record->t : tmax);
    return hit_left || hit_right;
}

file_internal bool 
intersect_ray_primitive(HitRecord  *record,
                        Ray        *ray,
                        Primitive  *primitive,
                        r32         tmin, 
                        r32        *tmax)
{
    HitRecord tmp_record{};
    bool hit_anything = false;
    
    if (g_intersection_look_up[primitive->type](&tmp_record, ray, primitive, tmin, tmax))
    {
        *tmax = tmp_record.t;
        *record = tmp_record;
        hit_anything = true;
    }
    
    return hit_anything;
}

file_internal bool 
intersect_ray_scene(HitRecord *record, 
                    Ray       *ray, 
                    Scene     *scene,
                    r32        tmin, 
                    r32        tmax)
{
    bool hit_anything = false;
    r32 closest_hit = tmax;
    
    if (scene->bvh_tree)
    {
        if (intersect_ray_bvh_node(record, ray, scene->bvh_tree, tmin, &closest_hit))
            hit_anything = true;
    }
    else
    {
        for (u32 i = 0; i < scene->primitives_count; ++i)
        {
            Primitive *primitive = &scene->primitives[i];
            if (intersect_ray_primitive(record, ray, primitive, tmin, &closest_hit)) 
                hit_anything = true;
        }
    }
    
    return hit_anything;
}

file_internal void
build_surrounding_box(Aabb *result, Aabb *box0, Aabb *box1)
{
    result->min = {
        fast_minf(box0->min.x, box1->min.x),
        fast_minf(box0->min.y, box1->min.y),
        fast_minf(box0->min.z, box1->min.z),
    };
    
    result->max = {
        fast_maxf(box0->max.x, box1->max.x),
        fast_maxf(box0->max.y, box1->max.y),
        fast_maxf(box0->max.z, box1->max.z),
    };
}

file_internal void
build_aabb_sphere(Aabb *aabb, PrimitiveSphere *sphere, r32 t0, r32 t1)
{
    aabb->min = v3_sub(sphere->origin, { sphere->radius, sphere->radius, sphere->radius });
    aabb->max = v3_add(sphere->origin, { sphere->radius, sphere->radius, sphere->radius });
}

file_internal void
build_aabb_dyn_sphere(Aabb *aabb, PrimitiveDynamicSphere *sphere, r32 t0, r32 t1)
{
    v3 center_t0 = get_sphere_center(sphere, t0);
    v3 center_t1 = get_sphere_center(sphere, t1);
    
    Aabb box0{};
    box0.min = v3_sub(center_t0, { sphere->radius, sphere->radius, sphere->radius });
    box0.max = v3_add(center_t0, { sphere->radius, sphere->radius, sphere->radius });
    
    Aabb box1{};
    box1.min = v3_sub(center_t1, { sphere->radius, sphere->radius, sphere->radius });
    box1.max = v3_add(center_t1, { sphere->radius, sphere->radius, sphere->radius });
    
    build_surrounding_box(aabb, &box0, &box1);
}

file_internal void
build_aabb_primitive(Aabb *aabb, Primitive *primitive, r32 t0, r32 t1)
{
    if (primitive->type == Primitive_Sphere)
    {
        build_aabb_sphere(aabb, &primitive->sphere, t0, t1);
    }
    else if (primitive->type == Primitive_DynamicSphere)
    {
        build_aabb_dyn_sphere(aabb, &primitive->dyn_sphere, t0, t1);
    }
    else if (primitive->type == Primitive_BvhNode)
    {
        *aabb = primitive->bvh_node.aabb;
    }
}

file_internal void
build_aabb_list(Aabb *aabb, Primitive *primitives, u32 count, r32 t0, r32 t1)
{
    if (count == 0) return;
    build_aabb_primitive(aabb, &primitives[0], t0, t1);
    
    Aabb tmp_box{};
    for (u32 i = 1; i < count; ++i)
    {
        build_aabb_primitive(&tmp_box, &primitives[i], t0, t1);
        build_surrounding_box(aabb, aabb, &tmp_box);
    }
}