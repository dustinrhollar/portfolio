#ifndef SPLICER_GAME_TEXTURE_H
#define SPLICER_GAME_TEXTURE_H

struct TextureComponent {
    // When a texture is initially created, it is
    // not immediately uploaded to the GPU. This is
    // set to true after that upload occurs.
    bool IsInitialized;
    
    // Image imformation
    ImageParameters Texture;
};

void InitializeTextureManager();
void ShutdownTextureManager();
// Loads a texture from a file. A texture component (Render related
// information) is created and saved. The id of the created tecture is
// returned.
u32 LoadTexture(jstring &filename);
jstring GetTextureFilename(u32 texture_id);
TextureComponent GetTextureComponent(u32 texture_id);

bool IsValidTexture(u32 texture_id);

// When a texture is created, it is initially placed into
// a buffer. This call uploads all textures in the buffer
// to the GPU.
void UploadBufferedTextures(VkCommandPool command_pool);

#endif
