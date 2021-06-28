
file_internal void 
camera_init(Camera *camera, CameraCreateInfo *info)
{
    camera->time0 = info->t0;
    camera->time1 = info->t1;
    
    r32 theta = degrees_to_radians(info->vfov);
    r32 h = tanf(theta / 2.0f);
    
    r32 viewport_height = 2.0f * h;
    r32 viewport_width  = info->aspect_ratio * viewport_height;
    r32 focal_length = 1.0f;
    
    camera->w = v3_norm(v3_sub(info->look_from, info->look_at));
    camera->u = v3_norm(v3_cross(info->up, camera->w));
    camera->v = v3_cross(camera->w, camera->u);
    
    camera->origin = info->look_from;
    camera->horizontal = v3_mulf(camera->u, info->focus_dist * viewport_width);
    camera->vertical   = v3_mulf(camera->v, info->focus_dist * viewport_height);
    
    // lower_left = orig - horit/2 - vert/2 - w*focus_dist
    camera->lower_left_corner = v3_sub(camera->origin, v3_divf(camera->horizontal, 2.0f));
    camera->lower_left_corner = v3_sub(camera->lower_left_corner, v3_divf(camera->vertical, 2.0f));
    camera->lower_left_corner = v3_sub(camera->lower_left_corner, v3_mulf(camera->w, info->focus_dist));
    camera->lens_radius = info->aperture / 2.0f;
}

file_internal void
camera_get_ray(Ray *ray, Camera *camera, r32 s, r32 t)
{
    v3 rd = v3_mulf(random_in_unit_disc(), camera->lens_radius);
    v3 offset = v3_add(v3_mulf(camera->u, rd.x), v3_mulf(camera->v, rd.y));
    
    ray->orig = v3_add(camera->origin, offset);
    ray->time = random_clamped(camera->time0, camera->time1);
    // dir = lower_left + horit*s + vert*t - origin - offset
    ray->dir = v3_add(camera->lower_left_corner, v3_mulf(camera->horizontal, s));
    ray->dir = v3_add(ray->dir, v3_mulf(camera->vertical, t));
    ray->dir = v3_sub(ray->dir, camera->origin);
    ray->dir = v3_sub(ray->dir, offset);
}