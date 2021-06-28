
#define WATER_SHADER_VERT "shaders/water_vert.glsl"
#define WATER_SHADER_FRAG "shaders/water_frag.glsl"

void water_renderer_init(
        WaterRenderer *renderer, 
        v2 reflection_fbo_dims,
        v2 refraction_fbo_dims,
        v4 reflection_clipping_plane, 
        v4 refraction_clipping_plane)
{
    shader_program_init(&renderer->shader, 2,
            ShaderStage_Vertex, WATER_SHADER_VERT,
            ShaderStage_Fragment, WATER_SHADER_FRAG);

    shader_program_bind(&renderer->shader);
    renderer_prepare_shader_uniforms(&renderer->shader);
    shader_program_set_int(&renderer->shader, "reflection_tex", 0);
    shader_program_set_int(&renderer->shader, "refraction_tex", 1);
    shader_program_set_int(&renderer->shader, "refraction_depth_tex", 2);
    shader_program_unbind(&renderer->shader);

    u32 width, height;
    PlatformGetWindowDims(&width, &height);

    fb_init(&renderer->reflection_fbo, reflection_fbo_dims.x, reflection_fbo_dims.y);
    fb_attach_texture(&renderer->reflection_fbo, Attachment_Color0);
    fb_attach_texture(&renderer->reflection_fbo, Attachment_Depth);
    fb_unbind(&renderer->reflection_fbo, width, height);

    fb_init(&renderer->refraction_fbo, refraction_fbo_dims.x, refraction_fbo_dims.y);
    fb_attach_texture(&renderer->refraction_fbo, Attachment_Color0);
    fb_attach_texture(&renderer->refraction_fbo, Attachment_DepthTexture);
    //fb_attach_texture(&renderer->refraction_fbo, Attachment_Depth);
    fb_unbind(&renderer->refraction_fbo, width, height);

    renderer->reflection_clip = reflection_clipping_plane;
    renderer->refraction_clip = reflection_clipping_plane;
}

void water_renderer_free(WaterRenderer *renderer)
{
    shader_program_free(&renderer->shader);
    fb_free(&renderer->reflection_fbo);
    fb_free(&renderer->refraction_fbo);
}


void water_gen_mesh(Water *water, WaterRenderer *renderer, u32 grid_length, r32 height)
{
    water->renderer = renderer;
    water->height = height;
    water->wave_time = 0.0f;
    RenderComponent *render_comp = &water->render_comp;

    u32 vertex_count = 0;
    {
        u32 sq_len = grid_length - 1;
        u32 grid_squares = sq_len * sq_len;
        vertex_count = grid_squares * 2 * 3; // 2 tris with 3 verts each
    }

    UniformBuffer primary;
    ub_init(&primary, vertex_count, sizeof(WaterVertex));

    for (u32 r = 0; r < grid_length; ++r)
    {
        for (u32 c = 0; c < grid_length; ++c)
        {
            // Evaluate the current "grid square"
            v2 corners[4] = {
                {{ (r32)(c + 0), (r32)(r + 0) }}, // (0) TL
                {{ (r32)(c + 0), (r32)(r + 1) }}, // (1) BL
                {{ (r32)(c + 1), (r32)(r + 0) }}, // (2) TR
                {{ (r32)(c + 1), (r32)(r + 1) }}, // (3) BR
            };

            WaterVertex wv[6];

            // Left Triangle

            wv[0].pos = corners[0];
            wv[0].indicators.xy = v2_sub(corners[1], corners[0]);
            wv[0].indicators.zw = v2_sub(corners[2], corners[0]);

            wv[1].pos = corners[1];
            wv[1].indicators.xy = v2_sub(corners[2], corners[1]);
            wv[1].indicators.zw = v2_sub(corners[0], corners[1]);
            
            wv[2].pos = corners[2];
            wv[2].indicators.xy = v2_sub(corners[0], corners[2]);
            wv[2].indicators.zw = v2_sub(corners[1], corners[2]);

            // Right Triangle

            wv[3].pos = corners[2];
            wv[3].indicators.xy = v2_sub(corners[1], corners[2]);
            wv[3].indicators.zw = v2_sub(corners[3], corners[2]);

            wv[4].pos = corners[1];
            wv[4].indicators.xy = v2_sub(corners[3], corners[1]);
            wv[4].indicators.zw = v2_sub(corners[2], corners[1]);
            
            wv[5].pos = corners[3];
            wv[5].indicators.xy = v2_sub(corners[2], corners[3]);
            wv[5].indicators.zw = v2_sub(corners[1], corners[3]);
            ub_ins(&primary, wv, 6); 
        }
    }

    render_component_init(render_comp, false, GL_TRIANGLES);
    render_comp->element_count = vertex_count;

    glBindVertexArray(render_comp->vao);

    glBindBuffer(GL_ARRAY_BUFFER, render_comp->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(WaterVertex) * vertex_count, primary.p, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(WaterVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(WaterVertex), (void*)sizeof(v2));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    ub_free(&primary);
}

#if 0
void water_gen_mesh(Water *water, WaterRenderer *renderer, u32 grid_length, r32 height)
{
    water->renderer = renderer;
    water->height = height;
    water->wave_time = 0.0f;
    RenderComponent *render_comp = &water->render_comp;

    u32 vertex_count = 0;
    {
        u32 bottom_two_rows = 2 * grid_length;
        u32 remaining_rows = grid_length - 2;
        u32 top_count = remaining_rows * (grid_length - 1) * 2;
        vertex_count = top_count + bottom_two_rows;
    }

    UniformBuffer primary;
    ub_init(&primary, vertex_count, sizeof(WaterVertex));

    UniformBuffer last_row;
    ub_init(&last_row, grid_length, sizeof(WaterVertex));

    u32 last_index = grid_length - 2;
    u32 vidx = 0;
    //u32 last_row_idx = 0;
    for (u32 r = 0; r < grid_length - 1; ++r)
    {
        for (u32 c = 0; c < grid_length - 1; ++c)
        {
            // Evaluate the current "grid square"
            v2 corners[4] = {
                {{ (r32)(c + 0), (r32)(r + 0) }}, // (0) TL
                {{ (r32)(c + 0), (r32)(r + 1) }}, // (1) BL
                {{ (r32)(c + 1), (r32)(r + 0) }}, // (2) TR
                {{ (r32)(c + 1), (r32)(r + 1) }}, // (3) BR
            };

            // storeTopLeftVertex
            // Winding Order: Tl -> BL -> BR
            // Indicators: xy = BL - TL, zw = BR - TL
            WaterVertex tl_vertex = {
                .pos = corners[0],
            };
            tl_vertex.indicators.xy = v2_sub(corners[1], corners[0]);
            tl_vertex.indicators.zw = v2_sub(corners[3], corners[0]);
            ub_ins(&primary, &tl_vertex, 1); 
            vidx++;

            // storeTopRightVertex
            // Winding Order: TR -> TL -> BR
            // Indicators: xy = TL - TR, zw = BR - TR
            if (r != last_index || c == last_index)
            {
                WaterVertex tr_vertex = {
                    .pos  = corners[2],
                };
                tr_vertex.indicators.xy = v2_sub(corners[0], corners[2]);
                tr_vertex.indicators.zw = v2_sub(corners[3], corners[2]);
                ub_ins(&primary, &tr_vertex, 1); 
                vidx++;
            }

            if (r == grid_length - 2)
            {
                if (c == 0)
                {
                    // store bottom left vertex
                    // Winding Order: BL -> TL -> BR
                    // Indicators: xy = TL - BL, zw = BR - BL
                    WaterVertex bl_vertex = {
                        .pos  = corners[1],
                    };
                    bl_vertex.indicators.xy = v2_sub(corners[0], corners[1]);
                    bl_vertex.indicators.zw = v2_sub(corners[3], corners[1]);
                    ub_ins(&last_row, &bl_vertex, 1);
                }

                // store bottom right vertex
                // Winding Order: BR -> TL -> TR
                // Indicators: xy = TL - BR, zw = TR - BR
                WaterVertex br_vertex = {
                    .pos  = corners[3],
                };
                br_vertex.indicators.xy = v2_sub(corners[0], corners[3]);
                br_vertex.indicators.zw = v2_sub(corners[2], corners[3]);

                //LogDebug("LAST ROW BUFFER INS");
                ub_ins(&last_row, &br_vertex, 1);
            }
        }
    }

    // Copy last row into actual vertex buffer...
    // Not sure if this is even needed, but it looks
    // like a single vertex needs to be places BEFORE
    // this row is copied in. Need to test this idea out
    ub_copy(&primary, &last_row);
    ub_free(&last_row);

    // GENERATE INDICES

    u32 index_count = (grid_length - 1) * (grid_length - 1) * 6;
    u32 row_len = (grid_length - 1) * 2;

    UniformBuffer index_buffer;
    ub_init(&index_buffer, index_count, sizeof(u32));

    // Store top section
    for (u32 r = 0; r < grid_length - 3; ++r)
    {
        for (u32 c = 0; c < grid_length - 1; ++c)
        {
            u32 top_left = (r * row_len) + (c * 2);
            u32 top_right = top_left + 1;
            u32 bottom_left = top_left + row_len;
            u32 bottom_right = bottom_left + 1;
            b8 right_handed = (c % 2) != (r % 2);

            u32 quad[6] = { 
                top_left, bottom_left, right_handed ? bottom_right : top_right, // left tri
                top_right, right_handed ? top_left : bottom_left, bottom_right // right tri
            };
            ub_ins(&index_buffer, quad, 6);
        }
    }

    // Store second last line
    u32 row = grid_length - 3;
    for (u32 c = 0; c < grid_length - 1; ++c)
    {
        u32 top_left = (row * row_len) + (c * 2);
        u32 top_right = top_left + 1;
        u32 bottom_left = top_left + row_len - c;
        u32 bottom_right = bottom_left + 1;
        b8 right_handed = (c % 2) != (row % 2);
        
        u32 quad[6] = { 
            top_left, bottom_left, right_handed ? bottom_right : top_right, // left tri
            top_right, right_handed ? top_left : bottom_left, bottom_right // right tri
        };
        ub_ins(&index_buffer, quad, 6);
    }

    // Store Last Line
    row = grid_length - 2;
    for (u32 c = 0; c < grid_length - 1; ++c)
    {
        u32 top_left = (row * row_len) + c;
        u32 top_right = top_left + 1;
        u32 bottom_left = top_left + grid_length;
        u32 bottom_right = bottom_left + 1;
        b8 right_handed = (c % 2) != (row % 2);
        
        u32 quad[6] = { 
            top_left, bottom_left, right_handed ? bottom_right : top_right, // left tri
            bottom_right, top_right, right_handed ? top_left : bottom_left // right tri
        };
        ub_ins(&index_buffer, quad, 6);
    }
    
    render_component_init(render_comp, true, GL_TRIANGLES);
    render_comp->element_count = index_count;

    glBindVertexArray(render_comp->vao);

    glBindBuffer(GL_ARRAY_BUFFER, render_comp->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(WaterVertex) * vertex_count, primary.p, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_comp->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * index_count, index_buffer.p, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(WaterVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(WaterVertex), (void*)sizeof(v2));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    ub_free(&index_buffer);
    ub_free(&primary);
}
#endif

void water_free(Water *water)
{
    render_component_free(&water->render_comp);
    water->renderer = 0;
}

void water_draw(Water *water)
{
    shader_program_bind(&water->renderer->shader);

    shader_program_set_float(&water->renderer->shader, "height",  water->height);

    water->wave_time += 0.0016f;
    shader_program_set_float(&water->renderer->shader, "wave_time",  water->wave_time);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fb_get_texture(&water->renderer->reflection_fbo, Attachment_Color0));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fb_get_texture(&water->renderer->refraction_fbo, Attachment_Color0));
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, fb_get_texture(&water->renderer->refraction_fbo, Attachment_DepthTexture));

    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    render_component_draw(&water->render_comp);

    glDisable(GL_BLEND);
    shader_program_unbind(&water->renderer->shader);
}

