
static r32  perlin_interpolated_noise(r32 x, r32 y);
static r32 perlin_smooth_noise(i32 x, i32 y);
static r32  perlin_interpolate(r32 a, r32 b, r32 blend);
static r32 perlin_random(i32 x, i32 y);

int noise2(int x, int y, int seed);
float lin_inter(float x, float y, float s);
float smooth_inter(float x, float y, float s);
float noise2d(float x, float y, int seed);


void perlin_init_seed(PerlinNoise *perlin, i32 seed, i32 octaves, r32 roughness, r32 amp)
{
    perlin->seed      = seed % 1000000000;
    perlin->octaves   = octaves;
    perlin->roughness = roughness;
    perlin->amp       = amp;
}

void perlin_init(PerlinNoise *perlin, i32 octaves, r32 roughness, r32 amp)
{
    perlin_init_seed(perlin, rand(), octaves, roughness, amp);
}

void perlin_seeded_coord(PerlinNoise *perlin, i32 *x, i32 *y)
{
    *x = *x * 49632 + perlin->seed;
    *y = *y * 325176 + perlin->seed;
}

r32 perlin_get_noise_2d(PerlinNoise *perlin, i32 x, i32 y)
{
    r32 total = 0.0f;
    r32 d = powf(2, perlin->octaves - 1);
    for (i32 i = 0; i < perlin->octaves; ++i)
    {
        r32 freq = powf(2, i) / d;
        r32 amp = powf(perlin->roughness, i) * perlin->amp;
        
        //total += perlin_interpolated_noise((r32)x * freq, (r32)y * freq) * amp;
        total += noise2d(x * freq, y * freq, perlin->seed) * amp;
        //total += noise2d(x / 8.f, y / 8.f, perlin->seed) * amp;
    }
    return total;
}

#if 0
static r32 perlin_interpolated_noise(r32 x, r32 y)
{
    i32 intX = (int)floor(x);
	r32 fracX = x - intX;
	i32 intY = (int)floor(y);
	r32 fracY = y - intY;

	r32 v1 = perlin_smooth_noise(intX, intY);
	r32 v2 = perlin_smooth_noise(intX + 1, intY);
	r32 v3 = perlin_smooth_noise(intX, intY + 1);
	r32 v4 = perlin_smooth_noise(intX + 1, intY + 1);
	r32 i1 = perlin_interpolate(v1, v2, fracX);
	r32 i2 = perlin_interpolate(v3, v4, fracX);
	return perlin_interpolate(i1, i2, fracY);
}

static r32 perlin_smooth_noise(i32 x, i32 y)
{
    r32 corners = 
        (perlin_random(x - 1, y - 1) + 
         perlin_random(x + 1, y - 1) + 
         perlin_random(x - 1, y + 1) + 
         perlin_random(x + 1, y + 1)) / 16.0f;
	
    r32 sides = 
        (perlin_random(x - 1, y) + 
         perlin_random(x + 1, y) + 
         perlin_random(x, y - 1) + 
         perlin_random(x, y + 1)) / 8.0f;
	
    r32 center = perlin_random(x, y) / 4.0f;
	
    return corners + sides + center;
}

static r32 perlin_interpolate(r32 a, r32 b, r32 blend)
{
    // TODO(Dustin):
    // - Alternate to this pi const here
    // - Avoid using cosine if possible
    static const r32 pi = 3.141592653589793238f;

    r32 theta = blend * pi;
	float f = (r32) ((1.0f - cosf(theta)) * 0.5f);
	return a * (1.0f - f) + b * f;
}

// Returns a random number between [-1, 1]
static r32 perlin_random(i32 x, i32 y)
{   
    r32 ran = rand() / (r32)RAND_MAX;
    return ran * 2.0f - 1.0f;
}

#else

static int hash[] = {208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
                     185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
                     9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
                     70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
                     203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
                     164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
                     228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
                     232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
                     193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
                     101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
                     135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
                     114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219};

int noise2(int x, int y, int seed)
{
    int tmp = hash[(y + seed) % 256];
    return hash[(tmp + x) % 256];
}

float lin_inter(float x, float y, float s)
{
    return x + s * (y-x);
}

float smooth_inter(float x, float y, float s)
{
    return lin_inter(x, y, s * s * (3-2*s));
}

float noise2d(float x, float y, int seed)
{
    int x_int = x;
    int y_int = y;
    float x_frac = x - x_int;
    float y_frac = y - y_int;
    int s = noise2(x_int, y_int, seed);
    int t = noise2(x_int+1, y_int, seed);
    int u = noise2(x_int, y_int+1, seed);
    int v = noise2(x_int+1, y_int+1, seed);
    float low = smooth_inter(s, t, x_frac);
    float high = smooth_inter(u, v, x_frac);
    return smooth_inter(low, high, y_frac);
}
#endif
