
// Proof for range of Perlin Function can be found here:
//https://digitalfreepen.com/2017/06/20/range-perlin-noise.html
//
// sqrt(N / 4)
// where N is the number of dimensions
#define PERLIN_3D_RANGE 0.86602540378f

// Hashing function (used for fast on-device pseudorandom numbers for randomness in noise)
DEVICE FORCE_INLINE
u32 Hash(unsigned int seed)
{
    seed = (seed + 0x7ed55d16) + (seed << 12);
    seed = (seed ^ 0xc761c23c) ^ (seed >> 19);
    seed = (seed + 0x165667b1) + (seed << 5);
    seed = (seed + 0xd3a2646c) ^ (seed << 9);
    seed = (seed + 0xfd7046c5) + (seed << 3);
    seed = (seed ^ 0xb55a4f09) ^ (seed >> 16);
    return seed;
}

// Random unsigned int for a grid coordinate [0, MAXUINT]
DEVICE FORCE_INLINE
u32 RandomIntGrid(i32 x, i32 y, i32 z, i32 seed = 0)
{
    return Hash((unsigned int)(x * 1723.0f + y * 93241.0f + z * 149812.0f + 3824 + seed));
}

DEVICE FORCE_INLINE
r32 Fade(r32 t) 
{
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); 
}

DEVICE FORCE_INLINE
r32 Grad(int hash, r32 x, r32 y, r32 z) {
    switch (hash & 0xF)
    {
		case 0x0: return x + y;
		case 0x1: return -x + y;
		case 0x2: return x - y;
		case 0x3: return -x - y;
		case 0x4: return x + z;
		case 0x5: return -x + z;
		case 0x6: return x - z;
		case 0x7: return -x - z;
		case 0x8: return y + z;
		case 0x9: return -y + z;
		case 0xA: return y - z;
		case 0xB: return -y - z;
		case 0xC: return y + x;
		case 0xD: return -y + z;
		case 0xE: return y - x;
		case 0xF: return -y - z;
		default: return 0; // never happens
    }
}

// Linearly interpolate between two float values
__device__ float Lerp(float a, float b, float ratio)
{
    return a * (1.0f - ratio) + b * ratio;
}

DEVICE
r32 SamplePerlinNoise(r32 x, r32 y, r32 z, r32 scale, i32 seed)
{
    r32 fseed = (r32)seed;
    
    // TODO(Dustin): Scale
    x *= scale;
    y *= scale;
    z *= scale;
    
    r32 ix = floorf(x);
    r32 iy = floorf(y);
    r32 iz = floorf(z);
    
    x -= ix;                                // FIND RELATIVE X,Y,Z
    y -= iy;                                // OF POINT IN CUBE.
    z -= iz;
    
    r32 u = Fade(x); // compute fade curves
    r32 v = Fade(y);
    r32 w = Fade(z);
    
    // Get the influence values from each corner of the cube
    r32 i000 = Grad(RandomIntGrid(ix, iy, iz, fseed), x, y, z);
    r32 i100 = Grad(RandomIntGrid(ix + 1.0f, iy, iz, fseed), x - 1.0f, y, z);
    r32 i010 = Grad(RandomIntGrid(ix, iy + 1.0f, iz, fseed), x, y - 1.0f, z);
    r32 i110 = Grad(RandomIntGrid(ix + 1.0f, iy + 1.0f, iz, fseed), x - 1.0f, y - 1.0f, z);
    r32 i001 = Grad(RandomIntGrid(ix, iy, iz + 1.0f, fseed), x, y, z - 1.0f);
    r32 i101 = Grad(RandomIntGrid(ix + 1.0f, iy, iz + 1.0f, fseed), x - 1.0f, y, z - 1.0f);
    r32 i011 = Grad(RandomIntGrid(ix, iy + 1.0f, iz + 1.0f, fseed), x, y - 1.0f, z - 1.0f);
    r32 i111 = Grad(RandomIntGrid(ix + 1.0f, iy + 1.0f, iz + 1.0f, fseed), x - 1.0f, y - 1.0f, z - 1.0f);
    
    // Interpolate
    float x00 = Lerp(i000, i100, u);
    float x10 = Lerp(i010, i110, u);
    float x01 = Lerp(i001, i101, u);
    float x11 = Lerp(i011, i111, u);
    
    float y0 = Lerp(x00, x10, v);
    float y1 = Lerp(x01, x11, v);
    
    float avg = Lerp(y0, y1, w);
    
    return avg;
}

GLOBAL
void PerlinKernel(r32 *fb, float3 dims, i32 seed, i32 n, r32 scale, r32 lacunarity, r32 decay)
{
    int i = (blockIdx.x * blockDim.x) + threadIdx.x;
    int j = (blockIdx.y * blockDim.y) + threadIdx.y;
    int pixel_index = j * dims.x + i;
    
    float3 pos = make_float3((r32)i / (r32)dims.x, (r32)j / (r32)dims.y, 0.0f);
    
    if ((i >= dims.x) || (j >= dims.y)) return;
    
    r32 acc = 0.0f;
    r32 amp = 1.0f;
    r32 range = 0.0f;
    for (i32 k = 0; k < n; ++k)
    {
        // 2D noise for now...
        acc += SamplePerlinNoise(pos.x * scale, pos.y * scale, pos.z * scale, 1.0f, ((k + 38) * 27389482)) * amp;
        
        scale *= lacunarity;
        range += PERLIN_3D_RANGE * amp;
        amp *= decay;
    }
    
    fb[pixel_index] = inv_lerp(-range, range, acc);
}

GLOBAL
void PerlinTurbulenceKernel(r32 *fb, float3 dims, i32 seed, i32 n, r32 scale, r32 lacunarity, r32 decay)
{
    int i = (blockIdx.x * blockDim.x) + threadIdx.x;
    int j = (blockIdx.y * blockDim.y) + threadIdx.y;
    int pixel_index = j * dims.x + i;
    
    float3 pos = make_float3((r32)i / (r32)dims.x, (r32)j / (r32)dims.y, 0.0f);
    
    if ((i >= dims.x) || (j >= dims.y)) return;
    
    r32 acc = 0.0f;
    r32 amp = 1.0f;
    r32 range = PERLIN_3D_RANGE;
    for (i32 k = 0; k < n; ++k)
    {
        // 2D noise for now...
        //acc += SamplePerlinNoise(pos.x * scale, pos.y * scale, pos.z * scale, 1.0f, ((k + 38) * 27389482)) * amp;
        acc += fabsf(SamplePerlinNoise(pos.x * scale, pos.y * scale, pos.z * scale, 1.0f, seed)) * amp;
        scale *= lacunarity;
        amp *= decay;
        range += PERLIN_3D_RANGE * amp;
    }
    
    // TODO(Dustin): I am not convinced this is working...
    fb[pixel_index] = inv_lerp(0.0f, range, acc);
}

//Predeclare these functions...TEMPORARY
void mprint(char *fmt, ...);
void mprinte(char *fmt, ...);

#include <Platform/Timer.h>

void PerlinKernelEntry(uchar4 *pbo, i32 texture_width, i32 texture_height)
{
    int nx = texture_width;
    int ny = texture_height;
    int tx = 16;
    int ty = 16;
    int num_pixels = nx * ny;
    size_t fb_size = sizeof(r32) * num_pixels;
    dim3 blocks(nx/tx+1, ny/ty+1);
    dim3 threads(tx,ty);
    
    // Image to write to
    r32* cudaimage = NULL;
    CheckCudaErrors(cudaMalloc((void**)&cudaimage, fb_size));
    
    Timer timer;
    timer.Begin();
    
    PerlinKernel<<<blocks, threads>>>(cudaimage, 
                                      make_float3(nx, ny, 0), 
                                      0x71889283, 
                                      8,
                                      5.9f,  // "Zoom" 
                                      2.0f,    // "Noisy"
                                      0.5f);
    
    CheckCudaErrors(cudaGetLastError());
    CheckCudaErrors(cudaDeviceSynchronize());
    
    r32 elapsed = timer.MiliSecondsElapsed();
    mprint("Ms elapsed: %lf\n", elapsed);
    
    SendImageToPboR<<<blocks, threads>>>(pbo, nx, ny, cudaimage);
    CheckCudaErrors(cudaGetLastError());
    CheckCudaErrors(cudaDeviceSynchronize());
    
    //retrieve image from GPU
    r32 *image = new r32[num_pixels];
    CheckCudaErrors(cudaMemcpy(image, cudaimage, nx * ny * sizeof(r32), cudaMemcpyDeviceToHost));
    
    CheckCudaErrors(cudaFree(cudaimage));
    cudaDeviceReset();
}