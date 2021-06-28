
file_internal void 
rt_entry(RaytracerSettings *settings)
{
    Camera *camera = settings->camera;
    r32 *image = settings->image;
    Scene *scene = settings->scene;
    
    u32 idx = 0;
    for (i32 j = settings->height - 1; j >= 0; --j)
    {
        for (i32 i = 0; i < settings->width; ++i)
        {
            i32 real_row = settings->height - 1 - j;
            idx = ((real_row * settings->width) + i) * 3;
            
            r32 u = (r32)i / (r32)(settings->width - 1);
            r32 v = (r32)j / (r32)(settings->height - 1);
            
            v3 color = V3_ZERO;
            for (u32 s = 0; s < settings->samples; ++s)
            {
                r32 u = (r32)(i + random()) / (r32)(settings->width - 1);
                r32 v = (r32)(j + random()) / (r32)(settings->height - 1);
                
                Ray ray;
                camera_get_ray(&ray, camera, u, v);
                color = v3_add(color, ray_color(&ray, scene, settings->depth));
            }
            
            r32 scale = 1.0f / (r32)settings->samples;
            color = v3_mulf(color, scale);
            
            // gamma correctioni
            color.r = sqrtf(color.r);
            color.g = sqrtf(color.b);
            color.b = sqrtf(color.g);
            
            color.r = fast_clampf(0.0f, 0.9999f, color.r);
            color.g = fast_clampf(0.0f, 0.9999f, color.g);
            color.b = fast_clampf(0.0f, 0.9999f, color.b);
            
#if 0
            image[idx++] = color.r;
            image[idx++] = color.g;
            image[idx++] = color.b;
#else
            image[idx+0] = color.r;
            image[idx+1] = color.g;
            image[idx+2] = color.b;
#endif
            
        }
    }
    
    LogDebug("Final index = %d", idx);
}

file_internal void 
rt_async(void *args)
{
    RtJob *job = (RtJob*)args;;
    LogInfo("Beginning Raytracing job...");
    
    RaytracerSettings *settings = job->settings;
    Camera *camera = settings->camera;
    r32 *image = settings->image;
    Scene *scene = settings->scene;
    
    i32 scan_x  = job->dims[0];
    i32 scan_y  = job->dims[1];
    i32 start_i = job->pos[0];
    i32 start_j = job->pos[1];
    
    i32 stop_j = fast_clamp(0, settings->height, start_j - scan_y);
    i32 stop_i = fast_clamp(0, settings->width,  start_i + scan_x);
    
    i32 idx = 0;
    for (i32 j = start_j; j >= stop_j; --j)
    {
        i32 real_row = settings->height - 1 - j;
        i32 row_pos = real_row * settings->width;
        
        for (i32 i = start_i; i < stop_i; ++i)
        {
            idx = (row_pos + i) * 3;
            
            r32 u = (r32)i / (r32)(settings->width - 1);
            r32 v = (r32)j / (r32)(settings->height - 1);
            
            v3 color = V3_ZERO;
            for (u32 s = 0; s < settings->samples; ++s)
            {
                r32 u = (r32)(i + random()) / (r32)(settings->width - 1);
                r32 v = (r32)(j + random()) / (r32)(settings->height - 1);
                
                Ray ray{};
                camera_get_ray(&ray, camera, u, v);
                color = v3_add(color, ray_color(&ray, scene, settings->depth));
            }
            
            r32 scale = 1.0f / (r32)settings->samples;
            color = v3_mulf(color, scale);
            
            // gamma correctioni
            color.r = sqrtf(color.r);
            color.g = sqrtf(color.b);
            color.b = sqrtf(color.g);
            
            color.r = fast_clampf(0.0f, 0.9999f, color.r);
            color.g = fast_clampf(0.0f, 0.9999f, color.g);
            color.b = fast_clampf(0.0f, 0.9999f, color.b);
            
            image[idx+0] = color.r;
            image[idx+1] = color.g;
            image[idx+2] = color.b;
        }
    }
    
    PlatformAtomicDec(job->counter);
    LogInfo("Finished job...");
}


file_internal void 
image_to_ppm(r32 *image, u32 width, u32 height)
{
    u32 count = 3 * width * height;
    int exp_size = sizeof(u8) * count + 512;
    
    PrettyBuffer buffer;
    pb_init(&buffer, exp_size);
    pb_write(&buffer, "P3\n%d %d\n255\n", width, height);
    
#if 1
    
    int idx = 0;
    for (i32 j = 0; j < height; ++j)
    {
        for (i32 i = 0; i < width; ++i)
        {
            
            int r = static_cast<int>(255.999 * image[idx + 0]);
            int g = static_cast<int>(255.999 * image[idx + 1]);
            int b = static_cast<int>(255.999 * image[idx + 2]);
            
            pb_write(&buffer, "%d %d %d\n", r, g, b);
            idx += 3;
        }
    }
    
#else
    
    for (int j = height-1; j >= 0; --j) {
        for (int i = 0; i < width; ++i) {
            auto r = double(i) / (width-1);
            auto g = double(j) / (height-1);
            auto b = 0.25;
            
            int ir = static_cast<int>(255.999 * r);
            int ig = static_cast<int>(255.999 * g);
            int ib = static_cast<int>(255.999 * b);
            
            pb_write(&buffer, "%d %d %d\n", ir, ig, ib);
        }
    }
    
#endif
    
    PlatformErrorType err = PlatformWriteBufferToFile("image.ppm", (u8*)buffer.start, buffer.size);
    assert(err == PlatformError_Success);
    
    pb_free(&buffer);
}
