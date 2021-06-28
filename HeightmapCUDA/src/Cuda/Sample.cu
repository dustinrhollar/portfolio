
// All it does is set the framebuffer color to a specific value
GLOBAL
void SimpleKernel(vec3 *fb, i32 max_x, i32 max_y)
{
    int i = (blockIdx.x * blockDim.x) + threadIdx.x;
    int j = (blockIdx.y * blockDim.y) + threadIdx.y;
    int pixel_index = j * max_x + i;
    
    if ((i >= max_x) || (j >= max_y)) return;
    
    fb[pixel_index] = vec3(1,0,0);
}

// pbo: OpenGl texture image
void SimpleEntry(uchar4 *pbo, i32 texture_width, i32 texture_height)
{
    int nx = texture_width;
    int ny = texture_height;
    int tx = 8;
    int ty = 8;
    int num_pixels = nx * ny;
    size_t fb_size = sizeof(vec3) * num_pixels;
    dim3 blocks(nx/tx+1, ny/ty+1);
    dim3 threads(tx,ty);
    
    // Image to write to
    vec3* cudaimage = NULL;
    CheckCudaErrors(cudaMalloc((void**)&cudaimage, fb_size));
    
    SimpleKernel<<<blocks, threads>>>(cudaimage, nx, ny);
    CheckCudaErrors(cudaGetLastError());
    CheckCudaErrors(cudaDeviceSynchronize());
    
    SendImageToPboRgb<<<blocks, threads>>>(pbo, nx, ny, cudaimage);
    CheckCudaErrors(cudaGetLastError());
    CheckCudaErrors(cudaDeviceSynchronize());
    
    CheckCudaErrors(cudaFree(cudaimage));
    cudaDeviceReset();
}
