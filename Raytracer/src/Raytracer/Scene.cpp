
file_internal void 
scene_init(Scene *scene, u32 cap)
{
    scene->primitives_cap = cap;
    scene->primitives_count = 0;
    scene->primitives = (Primitive*)malloc(sizeof(Primitive) * cap);
}

file_internal void 
scene_free(Scene *scene)
{
    free(scene->primitives);
    scene->primitives = 0;
    scene->primitives_cap = 0;
    scene->primitives_count = 0;
}

file_internal void 
scene_add(Scene *scene, struct Primitive *primitive)
{
    if (scene->primitives_count + 1 >= scene->primitives_cap)
    {
        u32 cap = scene->primitives_cap * 2;
        Primitive* prims = (Primitive*)malloc(sizeof(Primitive) * cap);
        for (u32 i = 0; i < scene->primitives_count; ++i)
            prims[i] = scene->primitives[i];
        scene->primitives_cap = cap;
    }
    
    scene->primitives[scene->primitives_count++] = *primitive;
}

file_internal void 
build_random_scene(Scene *scene, b8 motion_blur)
{
    const int count = 3;
    
    Material mat_ground, mat_reuse;
    Primitive sphere_reuse, dyn_reuse;
    
    make_lambertian(&mat_ground, { 0.5f, 0.5f, 0.5f });
    make_sphere(&sphere_reuse, { 0, -1000, 0 }, 1000, mat_ground);
    scene_add(scene, &sphere_reuse);
    
    for (i32 a = -count; a < count; ++a)
    {
        for (i32 b = -count; b < count; ++b)
        {
            r32 ChooseMat = random();
            v3 Center = { a * 0.9f * random(), 0.2f, b + 0.9f * random() };
            
            v3 Point = { 4, 0.2f, 0 };
            if (v3_mag(v3_sub(Center, Point)) > 0.9f)
            {
                if (ChooseMat < 0.8f)
                {
                    v3 Albedo = v3_mul(v3_random(), v3_random());
                    make_lambertian(&mat_reuse, Albedo);
                    if (motion_blur)
                    {
                        // dynamic spheres for motion blur
                        v3 RandY = { 0, random_clamped(0, 0.5f), 0 };
                        v3 Center2 = v3_add(Center, RandY);
                        make_dynamic_sphere(&dyn_reuse, Center, Center2, 0.0f, 1.0f, 0.2f, mat_reuse);
                        scene_add(scene, &dyn_reuse);
                    }
                    else
                    {
                        make_sphere(&sphere_reuse, Center, 0.2f, mat_reuse);
                        scene_add(scene, &sphere_reuse);
                    }
                }
                else if (ChooseMat < 0.95f)
                {
                    v3 Albedo = {
                        0.5f * (1.0f + random()),
                        0.5f * (1.0f + random()),
                        0.5f * (1.0f + random())
                    };
                    
                    r32 Fuzz = random_clamped(0.0f, 0.5f);
                    make_metal(&mat_reuse, Albedo, Fuzz);
                    make_sphere(&sphere_reuse, Center, 0.2f, mat_reuse);
                    scene_add(scene, &sphere_reuse);
                }
                else
                {
                    make_dielectric(&mat_reuse, 1.5f);
                    make_sphere(&sphere_reuse, Center, 0.2f, mat_reuse);
                    scene_add(scene, &sphere_reuse);
                }
            }
        }
    }
    
    make_dielectric(&mat_reuse, 1.5f);
    make_sphere(&sphere_reuse, { 0, 1, 0 }, 1.0f, mat_reuse);
    scene_add(scene, &sphere_reuse);
    
    make_lambertian(&mat_reuse, { 0.4f, 0.2f, 0.1f });
    make_sphere(&sphere_reuse, { -4.0f, 1.0f, 0.0f }, 1.0f, mat_reuse);
    scene_add(scene, &sphere_reuse);
    
    make_metal(&mat_reuse, { 0.7f, 0.6f, 0.5f }, 0.0f);
    make_sphere(&sphere_reuse, { 4.0f, 1.0f, 0.0f }, 1.0f, mat_reuse);
    scene_add(scene, &sphere_reuse);
    
    scene->bvh_tree = 0;
}