
#define SIMPLE_TERRAIN_VERT "shaders/simple_terrain_vert.glsl"
#define SIMPLE_TERRAIN_FRAG "shaders/simple_terrain_frag.glsl"
#define SIMPLE_TERRAIN_VERTEX_STRIDE (12 + 4 +4)

#define TERRAIN_SHADER_PROJ_VIEW_NAME   "proj_view"
#define TERRAIN_SHADER_LIGHT_DIR_NAME   "light_dir"
#define TERRAIN_SHADER_LIGHT_COLOR_NAME "light_col"
#define TERRAIN_SHADER_LIGHT_BIAS_NAME  "light_bias"

static v3 v3_calc_normal(v3 vert0, v3 vert1, v3 vert2);
static void terrain_mesh_gen_simple(
        RenderComponent *render_comp,
        PerlinNoise     *perlin,
        ColorGen        *color_gen,
        u32              terrain_width,
        u32              terrain_height);
static void terrain_mesh_gen_clever(
        RenderComponent *render_comp,
        PerlinNoise     *perlin,
        ColorGen        *color_gen,
        u32              terrain_width,
        u32              terrain_height);

// biomes: preset colors
void color_gen_init(ColorGen *color_gen, v3 *biomes, u32 biomes_count, r32 spread)
{
    color_gen->spread = spread;
    color_gen->half_spread = spread / 2.0f;
    color_gen->part = 1.0f / (biomes_count - 1.0f);
    color_gen->biomes_count = biomes_count;
    color_gen->biomes = MemAlloc(sizeof(v3) * biomes_count);
    for (u32 i = 0; i < biomes_count; ++i) color_gen->biomes[i] = biomes[i];
}

void color_gen_free(ColorGen *color_gen)
{
    if (color_gen->biomes) MemFree(color_gen->biomes);
    color_gen->spread = 0.0f;
    color_gen->half_spread = 0.0f;
    color_gen->part = 0.0f;
    color_gen->biomes = 0;
    color_gen->biomes_count = 0;
}

static c3 color_gen_interop_colors(c3 left, c3 right, r32 t)
{
    c3 result;

    result.r = lerp(left.r, right.r, t);
    result.g = lerp(left.g, right.r, t);
    result.b = lerp(left.b, right.r, t);

    return result;
}   

c3 color_gen_get_color(ColorGen *color_gen, r32 height, r32 amplitude)
{
    float value = (height + amplitude) / (amplitude * 2.0f);
    value = clamp(0.0f, 0.99999f, (value - color_gen->half_spread) * (1.0f / color_gen->spread));
    i32 first_biome = (i32)floor(value / color_gen->part);
    r32 blend = (value - (first_biome * color_gen->part)) / color_gen->part;
    return color_gen_interop_colors(color_gen->biomes[first_biome], color_gen->biomes[first_biome + 1], blend); 
}

void terrain_base_init(TerrainBase *base, PerlinNoise *perlin, ColorGen *color)
{
    base->perlin = perlin;
    base->color  = color;
}

void terrain_base_free(TerrainBase *base)
{
    base->perlin = 0;
    base->color  = 0;
}

void simple_terrain_gen_init(SimpleTerrainGenerator *terrain, PerlinNoise *perlin, ColorGen *color)
{
    terrain_base_init(&terrain->base, perlin, color);
    shader_program_init(&terrain->renderer.shader.handle, 2, 
            ShaderStage_Vertex,   SIMPLE_TERRAIN_VERT,
            ShaderStage_Fragment, SIMPLE_TERRAIN_FRAG);

    shader_program_bind(&terrain->renderer.shader.handle);
    renderer_prepare_shader_uniforms(&terrain->renderer.shader.handle);
    shader_program_unbind(&terrain->renderer.shader.handle);
}

void simple_terrain_gen_free(SimpleTerrainGenerator *terrain)
{
    terrain_base_free(&terrain->base);
}

static v3 v3_calc_normal(v3 vert0, v3 vert1, v3 vert2)
{
    v3 tan_a = v3_sub(vert1, vert0);
    v3 tan_b = v3_sub(vert2, vert0);
    v3 result = v3_cross(tan_a, tan_b);
    return v3_norm(result);
}

void simple_terrain_gen_generate(SimpleTerrainGenerator *generator, Terrain *result, u32 terrain_width, u32 terrain_height)
{
    // Initialize the terrain
    result->renderer = &generator->renderer;

#if 0
    terrain_mesh_gen_simple(
        &result->render_comp,
        generator->base.perlin,
        generator->base.color,
        terrain_width,
        terrain_height);
#else
    terrain_mesh_gen_clever(
        &result->render_comp,
        generator->base.perlin,
        generator->base.color,
        terrain_width,
        terrain_height);
#endif
}

void terrain_draw(Terrain *terrain, v4 clip_plane)
{
    ShaderProgram *shader = &terrain->renderer->shader.handle;
    shader_program_bind(shader);

    m4 model = M4_IDENTITY;

    shader_program_set_m4(shader, "model", model);
    shader_program_set_v4(shader, "clip", clip_plane);

    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
    render_component_draw(&terrain->render_comp);
}

static void terrain_mesh_gen_info(
        r32            **heights_result, 
        c3             **colors_result,
        PerlinNoise     *perlin,
        ColorGen        *color_gen,
        u32              terrain_width,
        u32              terrain_height)
{
    // Pre-Calculate the height + colors
    r32 *heights = MemAlloc(sizeof(r32) * terrain_width * terrain_height);
    c3  *colors  = MemAlloc(sizeof(c3)  * terrain_width * terrain_height);
    for (u32 r = 0; r < terrain_height; ++r)
    {
        u32 base = r * terrain_width;
        for (u32 c = 0; c < terrain_width; ++c)
        {
            r32 height = perlin_get_noise_2d(perlin, c, r);
            heights[base + c] = height;
        }
    }

    // Okay, this is really inefficient, but it works
    // The goal is to have all vertices between the 
    // -amp and amp. Need to scan over the mesh to find 
    // the min/max values and then remap all values
    // to the desired range
    u32 count = terrain_width * terrain_height;
    r32 max = -perlin->amp;
    r32 min =  perlin->amp;
    for (u32 i = 0; i < count; ++i)
    {
        if (heights[i] > max) max = heights[i];
        if (heights[i] < min) min = heights[i];
    }

    for (u32 i = 0; i < count; ++i)
    {
        heights[i] = remap(min, max, -perlin->amp, perlin->amp, heights[i]);
        assert(heights[i] >= -perlin->amp && heights[i] <= perlin->amp);
    }

    // Generate colors for the mesh
    for (u32 r = 0; r < terrain_height; ++r)
    {
        u32 base = r * terrain_width;
        for (u32 c = 0; c < terrain_width; ++c)
        {
            c3 color = color_gen_get_color(color_gen, heights[base+c], perlin->amp);
            colors[base + c] = color;
        }
    }
    
    *heights_result = heights;
    *colors_result = colors;
}

static void terrain_mesh_gen_clever(
        RenderComponent *render_comp,
        PerlinNoise     *perlin,
        ColorGen        *color_gen,
        u32              terrain_width,
        u32              terrain_height)
{
    r32 *heights = 0;
    c3 *colors = 0;
    terrain_mesh_gen_info(&heights, &colors, perlin, color_gen, terrain_width, terrain_height);
   
    u32 vertex_count = 0;
    {
        u32 bottom_two_rows = 2 * terrain_height;
        u32 remaining_rows = terrain_height - 2;
        u32 top_count = remaining_rows * (terrain_height - 1) * 2;
        vertex_count = top_count + bottom_two_rows;
    }

    LogDebug("Vertex Count for Clever Mesh: %d", vertex_count);

    struct Vertex
    {
        v3 pos, norm, col;
    };

    UniformBuffer primary;
    ub_init(&primary, vertex_count, sizeof(struct Vertex));

    UniformBuffer last_row;
    ub_init(&last_row, (terrain_height - 1) + 1, sizeof(struct Vertex));

    u32 last_index = terrain_height - 2;
    u32 vidx = 0;
    //u32 last_row_idx = 0;
    for (u32 r = 0; r < terrain_height - 1; ++r)
    {
        for (u32 c = 0; c < terrain_width - 1; ++c)
        {
            // Evaluate the current "grid square"
            // r = row, c = column, heights, colors
            v3 corners[4] = {
                {{ (r32)(c + 0), heights[((r + 0) * terrain_width) + (c + 0)], (r32)(r + 0) }},
                {{ (r32)(c + 0), heights[((r + 1) * terrain_width) + (c + 0)], (r32)(r + 1) }},
                {{ (r32)(c + 1), heights[((r + 0) * terrain_width) + (c + 1)], (r32)(r + 0) }},
                {{ (r32)(c + 1), heights[((r + 1) * terrain_width) + (c + 1)], (r32)(r + 1) }},
            };

            c3 color_corners[4] = {
                colors[((r + 0) * terrain_width) + (c + 0)],
                colors[((r + 1) * terrain_width) + (c + 0)],
                colors[((r + 0) * terrain_width) + (c + 1)],
                colors[((r + 1) * terrain_width) + (c + 1)],
            };

            b8 right_handed = (c % 2) != (r % 2);
            v3 tl_normal = v3_calc_normal(corners[0], corners[1], corners[right_handed ? 3 : 2]);
            v3 br_normal = v3_calc_normal(corners[2], corners[right_handed ? 0 : 1], corners[3]);

            // storeTopLeftVertex
            struct Vertex tl_vertex = {
                .pos = corners[0],
                .norm = tl_normal,
                .col = color_corners[0],
            };
            ub_ins(&primary, &tl_vertex, 1); 
            vidx++;

            // storeTopRightVertex
            if (r != last_index || c == last_index)
            {
                struct Vertex tr_vertex = {
                    .pos  = corners[2],
                    .norm = br_normal,
                    .col  = color_corners[2],
                };
                ub_ins(&primary, &tr_vertex, 1); 
                vidx++;
            }

            if (r == terrain_height - 2)
            {
                if (c == 0)
                {
                    // store bottom left vertex
                    struct Vertex bl_vertex = {
                        .pos  = corners[1],
                        .norm = tl_normal,
                        .col  = color_corners[1],
                    };
                    ub_ins(&last_row, &bl_vertex, 1);
                }

                // store bottom right vertex
                struct Vertex br_vertex = {
                    .pos  = corners[3],
                    .norm = br_normal,
                    .col  = color_corners[3],
                };

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

    u32 index_count = (terrain_height - 1) * (terrain_height - 1) * 6;
    u32 row_len = (terrain_height - 1) * 2;

    UniformBuffer index_buffer;
    ub_init(&index_buffer, index_count, sizeof(u32));

    // Store top section
    for (u32 r = 0; r < terrain_height - 3; ++r)
    {
        for (u32 c = 0; c < terrain_width - 1; ++c)
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
    u32 row = terrain_height - 3;
    for (u32 c = 0; c < terrain_width - 1; ++c)
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
    row = terrain_height - 2;
    for (u32 c = 0; c < terrain_width - 1; ++c)
    {
        u32 top_left = (row * row_len) + c;
        u32 top_right = top_left + 1;
        u32 bottom_left = top_left + terrain_height;
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * vertex_count, primary.p, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_comp->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * index_count, index_buffer.p, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)sizeof(v3));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)(2*sizeof(v3)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    MemFree(heights);
    MemFree(colors);

    ub_free(&index_buffer);
    ub_free(&primary);
}


static void terrain_mesh_gen_simple(
        RenderComponent *render_comp,
        PerlinNoise     *perlin,
        ColorGen        *color_gen,
        u32              terrain_width,
        u32              terrain_height)
{
    r32 *heights = 0;
    c3 *colors = 0;
    terrain_mesh_gen_info(&heights, &colors, perlin, color_gen, terrain_width, terrain_height);

    // Create the terrain mesh

    // Approach: "split" the vertices so they can each have a normal
    // Assume: Square grid
    u32 vertex_count = 0;
    {
        u32 sq_len = terrain_width - 1;
        u32 grid_squares = sq_len * sq_len;
        vertex_count = grid_squares * 2 * 3; // 2 tris with 3 verts each
    }

    struct Vertex
    {
        v3 pos;
        v3 norm;
        c3 col;
    } *vertices = MemAlloc(sizeof(struct Vertex) * vertex_count);
    
    // Generate the grid information
    u32 vidx = 0;
    for (u32 r = 0; r < terrain_height; ++r)
    {
        for (u32 c = 0; c < terrain_width; ++c)
        {
            v3 corners[4] = {
                {{ (r32)(c + 0), heights[((r + 0) * terrain_width) + (c + 0)], (r32)(r + 0) }},
                {{ (r32)(c + 0), heights[((r + 1) * terrain_width) + (c + 0)], (r32)(r + 1) }},
                {{ (r32)(c + 1), heights[((r + 0) * terrain_width) + (c + 1)], (r32)(r + 0) }},
                {{ (r32)(c + 1), heights[((r + 1) * terrain_width) + (c + 1)], (r32)(r + 1) }},
            };

            c3 color_corners[4] = {
                colors[((r + 0) * terrain_width) + (c + 0)],
                colors[((r + 1) * terrain_width) + (c + 0)],
                colors[((r + 0) * terrain_width) + (c + 1)],
                colors[((r + 1) * terrain_width) + (c + 1)],
            };

            v3 tl_normal = v3_calc_normal(corners[0], corners[1], corners[2]);
            v3 br_normal = v3_calc_normal(corners[2], corners[1], corners[3]);

            // Triangle 1: 0, 1, 2
            vertices[vidx++] = (struct Vertex){ .pos = corners[0], .norm = tl_normal, .col = color_corners[0] };
            vertices[vidx++] = (struct Vertex){ .pos = corners[1], .norm = tl_normal, .col = color_corners[1] };
            vertices[vidx++] = (struct Vertex){ .pos = corners[2], .norm = tl_normal, .col = color_corners[2] };

            // Triangle 2: 2, 1, 3
            vertices[vidx++] = (struct Vertex){ .pos = corners[2], .norm = br_normal, .col = color_corners[2] };
            vertices[vidx++] = (struct Vertex){ .pos = corners[1], .norm = br_normal, .col = color_corners[1] };
            vertices[vidx++] = (struct Vertex){ .pos = corners[3], .norm = br_normal, .col = color_corners[3] };
        }
    }

    // Initialize the terrain
    render_component_init(render_comp, false, GL_TRIANGLES);
    render_comp->element_count = vertex_count;

    LogDebug("Vertex Count: %d", vertex_count);

    glBindVertexArray(render_comp->vao);

    glBindBuffer(GL_ARRAY_BUFFER, render_comp->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * vertex_count, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)sizeof(v3));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)(2*sizeof(v3)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    MemFree(heights);
    MemFree(colors);
    MemFree(vertices);
}

#if 0

// Calculates the terrain of the using TRIANGLE_STRIPS

static void terrain_mesh_gen_tri_strip(
        RenderComponent *render_comp, 
        PerlinNoise     *perlin, 
        ColorGenerator  *color_gen, 
        u32              terrain_width, 
        u32              terrain_height)
{
    // Create the terrain mesh
    u32 vertex_count = terrain_width * terrain_height;
    u32 index_count = (terrain_width * terrain_height) + (terrain_width - 1) * (terrain_width - 2);

    struct Vertex
    {
        v3 pos;
        c3 col;
    } *vertices = MemAlloc(sizeof(struct Vertex) * vertex_count);
    u32 *indices = MemAlloc(sizeof(u32) * index_count);
    
    // Generate the grid information
    // Create the vertex list
    for (u32 r = 0; r < terrain_height; ++r)
    {
        u32 base = r * terrain_width;
        for (u32 c = 0; c < terrain_width; ++c)
        {
            r32 height = perlin_get_noise_2d(generator->base.perlin, c, r);
            c3 color = color_gen_get_color(generator->base.color, height, generator->base.perlin->amp);

            vertices[base + c].pos.x = (r32)c;
            vertices[base + c].pos.y = height;
            vertices[base + c].pos.z = (r32)r;
            vertices[base + c].col = color;
        }
    }

    // Create the indices list
    u32 i = 0;
    for (u32 r = 0; r < terrain_height - 1; r++)
    {
        if ((r & 1) == 0)
        { // even rows
            for (u32 c = 0; c < terrain_width; c++)
            {
                indices[i++] = (r + 0) * terrain_width + c;
                indices[i++] = (r + 1) * terrain_width + c;
            }
        }
        else
        {
            for (u32 c = terrain_width-1; c > 0; c--)
            {
                indices[i++] = c + (r + 1) * terrain_width;
                indices[i++] = c - 1 + (r + 0) * terrain_width;
            }
        }
        
        if ((terrain_height & 1) && terrain_height > 2)
        {
            indices[i++] = (terrain_height - 1) * terrain_width;
        }
    }

    // Initialize the terrain

    result->vertex_count = vertex_count;
    result->index_count = index_count;
    result->renderer = &generator->renderer;

    LogDebug("Vertex Count: %d", vertex_count);
    LogDebug("Index Count: %d", index_count);

    glGenVertexArrays(1, &result->vao); 
    glGenBuffers(1, &result->vbo);
    glGenBuffers(1, &result->ebo);

    glBindVertexArray(result->vao);

    glBindBuffer(GL_ARRAY_BUFFER, result->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * vertex_count, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, result->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * index_count, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)sizeof(v3));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    MemFree(vertices);
    MemFree(indices);
}


#endif
