#ifndef _CUDA_COMMON_H
#define _CUDA_COMMON_H

// Three channel copy
GLOBAL void SendImageToPboRgb(uchar4* PBOpos, i32 max_x, i32 max_y, vec3* image)
{
    int x = (blockIdx.x * blockDim.x) + threadIdx.x;
    int y = (blockIdx.y * blockDim.y) + threadIdx.y;
    int index = x + (y * max_x);
    
    if(x <= max_x && y <= max_y){
        
        vec3 color;
        
        color.x = image[index].x * 255.0f;
        color.y = image[index].y * 255.0f;
        color.z = image[index].z * 255.0f;
        
        if(color.x>255){
            color.x = 255;
        }
        
        if(color.y>255){
            color.y = 255;
        }
        
        if(color.z>255){
            color.z = 255;
        }
        
        // Each thread writes one pixel location in the texture (textel)
        PBOpos[index].w = 0;
        PBOpos[index].x = color.x;
        PBOpos[index].y = color.y;
        PBOpos[index].z = color.z;
    }
}

// Single channel copy
GLOBAL void SendImageToPboR(uchar4* PBOpos, i32 max_x, i32 max_y, r32* image)
{
    int x = (blockIdx.x * blockDim.x) + threadIdx.x;
    int y = (blockIdx.y * blockDim.y) + threadIdx.y;
    int index = x + (y * max_x);
    
    if(x <= max_x && y <= max_y)
    {
        r32 remap = image[index] * 255.0f;
        if(remap > 255.0f)
        {
            remap = 255.0f;
        }
        
        // Each thread writes one pixel location in the texture (textel)
        PBOpos[index].w = 0;
        PBOpos[index].x = remap;
        PBOpos[index].y = remap;
        PBOpos[index].z = remap;
    }
}


#endif //_CUDA_COMMON_H
