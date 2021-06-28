
#define DEFAULT_WINDOW_WIDTH  1920
#define DEFAULT_WINDOW_HEIGHT 1080

u8 g_render_wireframe = false;
r32 clear_color[3] = {0.0f,0.0f,0.0f};

void PlatformGetWindowDims(u32 *width, u32 *height)
{
    *width = DEFAULT_WINDOW_WIDTH;
    *height = DEFAULT_WINDOW_HEIGHT;
}


static void update_ctrl_bindings(HostWnd *wnd)
{
    for (Control ctrl = (Control)0; ctrl < Control_Count; ctrl += 1)
    {
        struct ControlState *state = g_control_states + ctrl;
        state->pressed = state->released = 0;
        
        if (host_wnd_is_key_pressed(wnd, g_control_bindings[ctrl]))
        {
            state->pressed = 1;
            state->down = 1;
        }
        
        if (host_wnd_is_key_released(wnd, g_control_bindings[ctrl]))
        {
            state->released = 1;
            state->down = 0;
        }
    }
}

typedef struct
{
    v3  pos;
    v3  up;
    v3  front;
    r32 speed;
    r32 sens;
    r32 yaw;
    r32 pitch;
    r32 fov;
} SampleCamera;

//void update_and_render(Renderer *renderer, SampleScene *sample, SampleCamera *cam)
void update_and_render(Renderer *renderer, Terrain *terrain, Water *water, SampleCamera *cam, Light *light)
{
    r32 delta = 0.0167 * cam->speed;
    r32 look_offset = 5 * cam->sens; // sensitivity * mouse_offset
    
    // Upddate the camera
    
    if (g_control_states[Control_MoveForward].pressed)
    {
        cam->pos = v3_add(cam->pos, v3_mulf(cam->front, delta));    
    }
    if (g_control_states[Control_MoveBack].pressed)
    {
        cam->pos = v3_sub(cam->pos, v3_mulf(cam->front, delta));    
    }
    if (g_control_states[Control_MoveLeft].pressed)
    {
        v3 right = v3_norm(v3_cross(cam->front, cam->up));
        cam->pos = v3_sub(cam->pos, v3_mulf(right, delta));    
    }
    if (g_control_states[Control_MoveRight].pressed)
    {
        v3 right = v3_norm(v3_cross(cam->front, cam->up));
        cam->pos = v3_add(cam->pos, v3_mulf(right, delta));    
    }
    
    //TODO(Dustin): Option for mouse/trackpad usage?
    b8 update_view = false;
    if (g_control_states[Control_LookUp].pressed)
    {
        update_view = true;
        cam->pitch += look_offset;
    }
    if (g_control_states[Control_LookDown].pressed)
    {
        update_view = true;
        cam->pitch -= look_offset;
    }
    if (g_control_states[Control_LookLeft].pressed)
    {
        update_view = true;
        cam->yaw -= look_offset;
    }
    if (g_control_states[Control_LookRight].pressed)
    {
        update_view = true;
        cam->yaw += look_offset;
    }
    
    if (update_view)
    {
        if (cam->pitch > 89.0f)  cam->pitch = 89.0f;
        if (cam->pitch < -89.0f) cam->pitch = -89.0f;
        r32 rad_yaw = degrees_to_radians(cam->yaw);
        r32 rad_pit = degrees_to_radians(cam->pitch);
        
        v3 front;
        front.x = cosf(rad_yaw) * cosf(rad_pit);
        front.y = sinf(rad_pit);
        front.z = sinf(rad_yaw) * cosf(rad_pit);
        cam->front = v3_norm(front);
    }
    
    // Update global uniforms
    ShaderGlobalData shader_data[2];
    shader_data[0].proj = m4_perspective(cam->fov, (r32)1920/(r32)1080, 0.01f, 1000.0f);
    shader_data[0].view = m4_look_at(cam->pos, v3_add(cam->pos, cam->front), cam->up);
    shader_data[0].cam_pos = cam->pos; 
    
    SampleCamera inverted_cam = *cam;
    inverted_cam.pos.y -= 2 * (cam->pos.y - water->height);
    inverted_cam.pitch *= -1.0f;
    
    { // update the front vector
        r32 rad_yaw = degrees_to_radians(inverted_cam.yaw);
        r32 rad_pit = degrees_to_radians(inverted_cam.pitch);
        
        v3 front;
        front.x = cosf(rad_yaw) * cosf(rad_pit);
        front.y = sinf(rad_pit);
        front.z = sinf(rad_yaw) * cosf(rad_pit);
        inverted_cam.front = v3_norm(front);
    }
    
    shader_data[1].proj = m4_perspective(inverted_cam.fov, (r32)1920/(r32)1080, 0.01f, 1000.0f);
    shader_data[1].view = m4_look_at(inverted_cam.pos, v3_add(inverted_cam.pos, inverted_cam.front), inverted_cam.up);
    shader_data[1].cam_pos  = inverted_cam.pos; 
    
    // Draw the scene
    
    glBindBuffer(GL_UNIFORM_BUFFER, renderer->global_data);
    void *ubo = glMapBufferRange(GL_UNIFORM_BUFFER, 0, renderer->ubo_gd_size, GL_MAP_WRITE_BIT);
    memcpy(ubo, &shader_data[0], sizeof(ShaderGlobalData));
    memcpy((char*)ubo + renderer->ubo_min_alignment, &shader_data[1], sizeof(ShaderGlobalData));
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glEnable(GL_CLIP_DISTANCE0);
    
    // Render Pass: Water Reflection
    
    glBindBufferRange(GL_UNIFORM_BUFFER, 
                      SHADER_UBO_BINDING_GLOBAL, 
                      renderer->global_data, 
                      renderer->ubo_min_alignment, 
                      sizeof(ShaderGlobalData));
    
    glBindBufferRange(GL_UNIFORM_BUFFER, 
                      SHADER_UBO_BINDING_LIGHTING, 
                      renderer->light_data, 
                      0, 
                      renderer->ubo_lt_size);
    
    fb_bind(&water->renderer->reflection_fbo);
    x11_opengl_begin_frame(clear_color);
    
    terrain_draw(terrain,
                 (v4){{ 0.0f, 1.0f, 0.0f, -water->height }});
    
    fb_unbind(&water->renderer->reflection_fbo, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    
    // Render Pass: Water Refraction
    
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, renderer->global_data, 0, sizeof(ShaderGlobalData));   
    
    fb_bind(&water->renderer->refraction_fbo);
    x11_opengl_begin_frame(clear_color);
    
    terrain_draw(terrain,
                 (v4){{ 0.0f, -1.0f, 0.0f, water->height + 1.0f }});
    
    fb_unbind(&water->renderer->refraction_fbo, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    
    // Render Pass: Final
    
    glDisable(GL_CLIP_DISTANCE0);
    x11_opengl_begin_frame(clear_color);
    
    // Not all ogl drivers actually disable the clip distance
    // so give the clip variable a really large value just in case...
    terrain_draw(
                 terrain,
                 (v4){{ 0.0f, -1.0f, 0.0f, 10000000.f }});
    water_draw(water);
}

int main() 
{
    // Use the Logger to catch seg faults so that it can report
    // a stack trace
    struct sigaction sig;
    sig.sa_sigaction = (void*)log_seg_handler;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = SA_RESTART|SA_SIGINFO;
    sigaction(SIGSEGV, &sig, NULL);
    sigaction(SIGUSR1, &sig, NULL);
    
    srand(time(NULL));
    
    u64   app_backing_memory_size = _16MB;
    void *app_backing_memory = 0;
    X11RequestMemory(&app_backing_memory, app_backing_memory_size);
    SysMemoryInit(app_backing_memory, app_backing_memory_size);
    
    HostWnd *client = 0;
    host_wnd_init(&client, 1920, 1080, "Maple Genetics");
    host_wnd_set_active(client);
    
    // Load glad library?
    if (!gladLoadGL())
    {
        LogFatal("Hey....uh...glad failed to load opengl functions");
    }
    
    Renderer renderer;
    renderer_init(&renderer);
    
    SampleCamera cam = {
        .pos   = {{ 10.0f, 5.0f, 10.0f }},
        .up    = {{ 0.0f, 1.0f,  0.0f }},
        .front = {{ 0.0f, 0.0f, -1.0f }},
        .speed = 15.5f,
        .sens  = 0.6f,
        .yaw   = 45.0f,
        .pitch = 0.0f,
        .fov   = 45.0f,
    };
    
    // Generate terrain
    
    r32 color_spread = 0.65f;
    static const u32 c_biome_colors_count = 5;
    c3 biome_colors[5] = {
        {{ (201.f / 255.f), (178.f / 255.f), (99.f / 255.) }},
        {{ (135.f / 255.f), (184.f / 255.f), (82.f / 255.) }},
        {{ (80.f / 255.f),  (171.f / 255.f), (93.f / 255.) }},
        {{ (120.f / 255.f), (120.f / 255.f), (120.f / 255.) }},
        {{ (200.f / 255.f), (200.f / 255.f), (210.f / 255.) }},
    };
    
    Light sun;
    sun.dir = (v3){
        .x = 0.3f,
        .y = -1.0f,
        .z = 0.5f,
    };
    
    sun.color = (v3){
        .x = 1.0f,
        .y = 0.8f,
        .z = 0.8f,
    };
    
    sun.bias = (v2){
        .x = 0.3f,
        .y = 0.8f,
    };
    
    sun.dir = v3_norm(sun.dir);
    
    u32 terrain_size = 100;
    r32 amplitude = 5;
    r32 roughness = 0.35f;
    u32 octaves = 5;
    
    PerlinNoise perlin;
    perlin_init(&perlin, octaves, roughness, amplitude);
    
    ColorGen color_gen;
    color_gen_init(&color_gen, biome_colors, c_biome_colors_count, color_spread);
    
    SimpleTerrainGenerator terrain_gen;
    simple_terrain_gen_init(&terrain_gen, &perlin, &color_gen);
    
    Terrain terrain;
    simple_terrain_gen_generate(&terrain_gen, &terrain, terrain_size, terrain_size);
    
    WaterRenderer water_renderer;
    water_renderer_init(
                        &water_renderer,
                        (v2){ .x = DEFAULT_WINDOW_WIDTH,      .y = DEFAULT_WINDOW_HEIGHT      },
                        (v2){ .x = DEFAULT_WINDOW_WIDTH/2.0f, .y = DEFAULT_WINDOW_HEIGHT/2.0f },
                        (v4){ .x = 0, .y = 0, .z = 0, .w = 0 },
                        (v4){ .x = 0, .y = 0, .z = 0, .w = 0 });
    
    Water water;
    water_gen_mesh(&water, &water_renderer, terrain_size, -0.5f);
    
    // Go ahead and update lighting data
    // this only needs to be done a single time
    
    ShaderLightingData sld = {
        .light_color = sun.color,
        .light_dir   = sun.dir,
        .light_bias  = sun.bias,
    };
    
    glBindBuffer(GL_UNIFORM_BUFFER, renderer.light_data);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, renderer.ubo_lt_size, &sld);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    while (true)
    {
        if (!host_wnd_msg_loop(client))
        {
            break;
        }
        
        update_ctrl_bindings(client);
        if (g_control_states[Control_CloseApp].pressed) break;
        if (g_control_states[Control_RenderWireframe].pressed)
        {
            g_render_wireframe = !g_render_wireframe;
        }
        
        if (!g_render_wireframe)
        {
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        }
        else
        {
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        }
        
        update_and_render(&renderer, &terrain, &water, &cam, &sun);
        
        x11_opengl_end_frame(client);
    }
    
    //sample_free(&sample);
    renderer_free(&renderer);
    host_wnd_free(&client);
    SysMemoryFree();
    X11ReleaseMemory(&app_backing_memory, app_backing_memory_size);
    
    return 0;
}
