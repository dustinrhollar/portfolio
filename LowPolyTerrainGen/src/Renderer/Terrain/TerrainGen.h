#ifndef _TERRAIN_GEN
#define _TERRAIN_GEN

typedef struct 
{
    r32  spread;
    r32  half_spread;
    r32  part;
    v3  *biomes;
    u32  biomes_count;
} ColorGen;

typedef struct 
{
    ShaderProgram handle;
    v3            light_dir;
    v3            light_col;
    v2            light_bias;
} TerrainShader;

typedef struct 
{
    PerlinNoise *perlin;
    ColorGen    *color;
} TerrainBase;

typedef struct
{
    TerrainShader shader;
    b8            has_indices;
} TerrainRenderer;

typedef struct 
{
    TerrainBase     base;
    TerrainRenderer renderer;
} SimpleTerrainGenerator;

typedef struct 
{
    TerrainRenderer *renderer;
    RenderComponent  render_comp;
} Terrain;

// biomes: preset colors
void color_gen_init(ColorGen *color_gen, v3 *biomes, u32 biomes_count, r32 spread);
void color_gen_free(ColorGen *color_gen);
c3 color_gen_get_color(ColorGen *color_gen, r32 height, r32 amplitude);


void terrain_base_init(TerrainBase *base, PerlinNoise *perlin, ColorGen *color);
void terrain_base_free(TerrainBase *base);

void simple_terrain_gen_init(SimpleTerrainGenerator *terrain, PerlinNoise *perlin, ColorGen *color);
void simple_terrain_gen_free(SimpleTerrainGenerator *terrain);
void simple_terrain_gen_generate(SimpleTerrainGenerator *generator, Terrain *result, u32 terrain_width, u32 terrain_height);

void terrain_draw(Terrain *terrain, v4 clip_plane);

#endif //_TERRAIN_GEN
