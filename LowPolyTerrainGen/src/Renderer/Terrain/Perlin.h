#ifndef _PERLIN_H
#define _PERLIN_H

typedef struct
{
    i32 seed;
    r32 roughness;
    i32 octaves;
    r32 amp;
} PerlinNoise;

void perlin_init_seed(PerlinNoise *perlin, i32 seed, i32 octaves, r32 roughness, r32 amp);
void perlin_init(PerlinNoise *perlin, i32 octaves, r32 roughness, r32 amp);
r32 perlin_get_noise_2d(PerlinNoise *perlin, i32 x, i32 y);

#endif
