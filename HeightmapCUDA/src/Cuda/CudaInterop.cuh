#ifndef _CUDA_INTEROP_H
#define _CUDA_INTEROP_H

enum class ImageType : u8
{
    R,
    G,
    B,
    A,
    RG,
    RGB,
    RGBA,
    
    Count,
    Unknown = Count,
};

enum class NoiseDim : u8
{
    One,
    Two,
    Three,
    Four,
    
    Count,
    Unknown = Count
};

void SimpleEntry(uchar4 *PBOpos, i32 max_x, i32 max_y);
void PerlinKernelEntry(uchar4 *pbo, i32 texture_width, i32 texture_height);


#endif //_CUDA_INTEROP_H
