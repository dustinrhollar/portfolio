
// Used for STBI_REALLOC
void *TextureRealloc(void *ptr, size_t size);

#define STBI_MALLOC(x)    palloc(x)
#define STBI_FREE(x)      pfree(x)
#define STBI_REALLOC(p,s) prealloc(p,s)

// For now, let's say only 32 textures can be loaded at once
file_global const u32 MAX_TEXTURES = 32;

// allows for 2^24 unique textures. I seriously doubt there will ever be
// that many loaded :p
static const u32 TEXTURE_INDEX_BITS = 24;
static const u32 TEXTURE_INDEX_MASK = (((unsigned __int32)1)<<TEXTURE_INDEX_BITS)-1;

static const u32 TEXTURE_GENERATION_BITS = 8;
static const u32 TEXTURE_GENERATION_MASK = (((unsigned __int32)1)<<TEXTURE_GENERATION_BITS)-1;

#define MAX(x,y) ((x) >= (y) ? (x) : (y))
#define MIN(x,y) ((x) <= (y) ? (x) : (y))

struct TextureEntity {
    u32 Id;
};

// An entity with an id of 0 is an invalid entity
#define INVALID_TEXTURE_ENTITY 0

// Used to determine uniqueness
file_global u8 TextureGenerations[MAX_TEXTURES];
// Used to retrieve renderinfo for that texture
file_global TextureComponent TextureRegistry[MAX_TEXTURES];
// used to retrive system information for a texture (filename)
file_global jstring TextureSystemInfo[MAX_TEXTURES];
file_global HashTable<jstring, TextureEntity> TextureSystemTable;

// Next index to insert
file_global u32 LastTextureEntityIndex = 0;

// List for indices ready for re-use, treated as a stack for quick push/pop operations
file_global LinkedList<u32> TextureFreeIndices;
// minimum available inidces before indices can be re-used
file_global size_t MinAllowedTextureFreeIndices = 8;

// Uploading a texture is deferred to the next frame.
// The following hold partially created textures.
struct PartialTexture {
    TextureEntity Entity;
    
    // Buffered data
    BufferParameters StagingBuffer;
    
    // texture info ?
    //void *Data;
    int Width, Height, Channels;
};

// NOTE(Dustin): this is a rather naive approach to this. might want something
// better in the future.
file_global PartialTexture TextureUploadBuffer[MAX_TEXTURES];
file_global u32 TextureUploadCount = 0;

// Useful forward declarations
file_internal TextureEntity GenerateTextureEntity(u32 index, u32 generation);

inline u32 TextureEntityToIndex(TextureEntity entity) {
    return (entity.Id & TEXTURE_INDEX_MASK);
}

inline u32 TextureEntityToGeneration(TextureEntity entity) {
    return (entity.Id >> TEXTURE_INDEX_BITS) & TEXTURE_GENERATION_MASK;
}

file_internal TextureEntity GenerateTextureEntity(u32 index, u32 generation) {
    TextureEntity entity;
    u32 idx   = (index << TEXTURE_INDEX_MASK);
    u32 gen   = ((generation >> TEXTURE_INDEX_BITS) & TEXTURE_GENERATION_MASK);
    entity.Id = ((generation << TEXTURE_INDEX_BITS)) | (index);
    return entity;
}

// Formality, and maybe useful in the future
void InitializeTextureManager() {
    
    TextureSystemTable = HashTable<jstring, TextureEntity>(MAX_TEXTURES);
    
    // Set all textures in the registry to be inactive
    for (int i = 0; i < MAX_TEXTURES; ++i) {
        TextureRegistry[i].IsInitialized = false;
    }
}

void ShutdownTextureManager() {
    TextureFreeIndices.Reset();
    TextureSystemTable.Reset();
    
    // Just in case there were pending resources
    // that did not get uploaded
    PartialTexture pt;
    
    while (TextureUploadCount > 0)
    {
        pt = TextureUploadBuffer[TextureUploadCount-1];
        vk::DestroyVmaBuffer(pt.StagingBuffer.Handle, pt.StagingBuffer.Memory);
        
        TextureUploadCount--;
    }
    
    // Destroy all of the textures in the registry
    for (int i = 0; i < MAX_TEXTURES; ++i) {
        if (TextureRegistry[i].IsInitialized) {
            TextureComponent tc = TextureRegistry[i];
            
            vk::DestroyImageSampler(tc.Texture.Sampler);
            vk::DestroyImageView(tc.Texture.View);
            vk::DestroyVmaImage(tc.Texture.Handle, tc.Texture.Memory);
            
            TextureRegistry[i].IsInitialized = false;
        }
    }
    
    for (int i = 0; i < MAX_TEXTURES; ++i) {
        TextureSystemInfo[i].Clear();
    }
}

u32 LoadTexture(jstring &filename) {
    
    // Linear search to determine if the texture is already loaded.
    TextureEntity entity;
    if (TextureSystemTable.Get(filename, &entity))
    {
        return entity.Id;
    }
    
    u32 index = INVALID_TEXTURE_ENTITY;
    if (TextureFreeIndices.Size() >= MinAllowedTextureFreeIndices) {
        TextureFreeIndices.Pop(index);
    }
    else {
        u8 default_idx = 0;
        index = LastTextureEntityIndex;
        TextureGenerations[index] = (default_idx);
        assert(index < (((unsigned __int32)1)<<TEXTURE_INDEX_BITS));
        
        LastTextureEntityIndex++;
    }
    
    entity = GenerateTextureEntity(index, TextureGenerations[index]);
    
    // Load the texture using stb...probably
    PartialTexture pt = {};
    pt.Entity = entity;
    
    unsigned char *data = stbi_load(filename.GetCStr(), &pt.Width, &pt.Height, &pt.Channels, STBI_rgb_alpha);
    VkDeviceSize image_size = pt.Width * pt.Height * 4;
    
    if (!data) {
        // NOTE(Dustin): Silent failure...
        printf("Failed to load texture from file %s!\n", filename.GetCStr());
        return 0;
    }
    
    // size, usage, properties
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size        = image_size;
    buffer_create_info.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    // prop: VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    VmaAllocationCreateInfo vma_create_info = {};
    vma_create_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    
    pt.StagingBuffer.Size = image_size;
    vk::CreateVmaBuffer(buffer_create_info,
                        vma_create_info,
                        pt.StagingBuffer.Handle,
                        pt.StagingBuffer.Memory,
                        pt.StagingBuffer.AllocationInfo);
    
    void *mapped_memory;
    vk::VmaMap(&mapped_memory, pt.StagingBuffer.Memory);
    {
        memcpy(mapped_memory, data, pt.StagingBuffer.Size);
    }
    vk::VmaUnmap(pt.StagingBuffer.Memory);
    
    // the texture memory since it has been copied to the staging buffer
    stbi_image_free(data);
    
    // Add the texture to the various containers.
    TextureUploadBuffer[TextureUploadCount++] = pt;
    TextureSystemInfo[index] = filename;
    TextureSystemTable.Insert(filename, pt.Entity);
    
    // NOTE(Dustin): Don't need to add a texture component to the registry yet...
    
    return entity.Id;
}

void UploadBufferedTextures(VkCommandPool command_pool) {
    
    PartialTexture pt;
    while (TextureUploadCount > 0)
    {
        pt = TextureUploadBuffer[TextureUploadCount-1];
        TextureUploadCount--;
        
        u32 idx = TextureEntityToIndex(pt.Entity);
        
        if (!IsValidTexture(pt.Entity.Id)) {
            // For some reason the texture was destroyed.
            vk::DestroyVmaBuffer(pt.StagingBuffer.Handle, pt.StagingBuffer.Memory);
            continue;
        }
        
        TextureComponent tex_comp = {};
        
        // Calculate mip levels
        u32 mip_levels = ((u32)floor(log2(MAX(pt.Width, pt.Height)))) + 1;
        
        // Create the Image for the component
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType     = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width  = (u32)(pt.Width);
        imageInfo.extent.height = (u32)(pt.Height);
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = mip_levels;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage         = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags         = 0; // Optional
        
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        
        // Create the image and copy the data into it.
        vk::CreateVmaImage(imageInfo, alloc_info,
                           tex_comp.Texture.Handle,
                           tex_comp.Texture.Memory,
                           tex_comp.Texture.AllocationInfo);
        
        vk::TransitionImageLayout(command_pool,
                                  tex_comp.Texture.Handle,
                                  VK_FORMAT_R8G8B8A8_SRGB,
                                  VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  mip_levels);
        
        vk::CopyBufferToImage(command_pool,
                              pt.StagingBuffer.Handle,
                              tex_comp.Texture.Handle,
                              pt.Width, pt.Height);
        
        // Image is transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        // while generating mipmaps
        
        vk::DestroyVmaBuffer(pt.StagingBuffer.Handle, pt.StagingBuffer.Memory);
        
        vk::GenerateMipmaps(command_pool,
                            tex_comp.Texture.Handle, VK_FORMAT_R8G8B8A8_SRGB,
                            pt.Width, pt.Height, mip_levels);
        
        // Create the image view for the texture
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image                           = tex_comp.Texture.Handle;
        viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format                          = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = mip_levels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;
        
        tex_comp.Texture.View = vk::CreateImageView(viewInfo);
        
        // Create the Texture Sampler
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter               = VK_FILTER_LINEAR;
        samplerInfo.minFilter               = VK_FILTER_LINEAR;
        samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable        = VK_TRUE;
        samplerInfo.maxAnisotropy           = 16;
        samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable           = VK_FALSE;
        samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias              = 0.0f;
        samplerInfo.minLod                  = 0.0f;
        samplerInfo.maxLod                  = (float)mip_levels;
        
        tex_comp.Texture.Sampler = vk::CreateImageSampler(samplerInfo);
        
        // Add the initialized Texture Component to the Registry
        tex_comp.IsInitialized = true;
        TextureRegistry[idx] = tex_comp;
        
        
    }
}

// True if valid (generations match), false otherwise
bool IsValidTexture(u32 id) {
    TextureEntity entity = {id};
    u32 idx = TextureEntityToIndex(entity);
    u32 gen = TextureEntityToGeneration(entity);
    
    return (TextureGenerations[idx] == gen);
}

jstring GetTextureFilename(u32 id) {
    TextureEntity entity = {id};
    
    // if the texture is not valid, return an empty string
    return (IsValidTexture(entity.Id)) ? TextureSystemInfo[TextureEntityToIndex(entity)] : jstring();
}

TextureComponent GetTextureComponent(u32 id) {
    TextureEntity entity = {id};
    
    // NOTE(Dustin): Silent Failure if the texture is no longer valid
    TextureComponent comp = {};
    if (IsValidTexture(entity.Id))
        comp = TextureRegistry[TextureEntityToIndex(entity)];
    return comp;
}
