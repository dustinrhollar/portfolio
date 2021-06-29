#ifndef RENDERER_H
#define RENDERER_H

enum ETextureType
{
    TEXTURE_TYPE_DIFFUSE,
    TEXTURE_TYPE_SPECULAR,
};

typedef struct
{
    unsigned int width;
    unsigned int height;
    unsigned int id;
    ETextureType type; // type of the texture. Useful for Model rendering
} Texture;

typedef enum 
{
    RENDER_MODE_FORWARD_RENDERING,
    RENDER_MODE_DEFERRED_RENDERING,
} RenderMode;

// Shader pipeline for Rendering
typedef struct 
{
    Shader     *geometryShader;
    
    // Material Properties
    // -------------------------
    // supported textures
    bool hasDiffuse;
    bool hasNormal;
    bool hasAlbedo;
    
    Texture diffuseTexture;
    Texture normalTexture;
    Texture albedoTexture;
    
    float shininess;
} Pipeline;

// Maintains the list of ALL active pipelines in the program
// TODO(Dustin): Use a HashTable instead
// TODO(Dustin): Use a memory arena?
typedef struct
{
    int size;
    int cap;
    Pipeline *pipes;
} PipelineList;
extern Pipeline *GlobalPipelineManager;

// Inherits from RenderInfo
typedef struct 
{
    Shader  *lightingShader; // Deferred rendering. Computes lighting info.
    Shader  *lightBoxShader; // Deferred rendering. Shades.
    
    // Framebuffer Textures
    Texture positionColorBuffer;
    Texture specColorBuffer;
    Texture normalColorBuffer;
    Texture depthBuffer;
    
    unsigned int framebuffer;
    unsigned int attachments[3]; // color attachments
} DeferredRenderInfo;

// Texture Constructors
void createTexture(Texture *texture, unsigned int width, unsigned int height,
                   GLint internalformat, GLenum format, GLenum type, char *data);
void createFramebufferTexture(Texture *texture, unsigned int width, unsigned int height,
                              GLint internalformat, GLenum format, GLenum type,
                              GLenum attachment);
void createRenderbufferTexture(Texture *texture, unsigned int width, unsigned int height);

void FreeTexture(Texture *texture);

// TODO(Dustin): Adjust the PipelineManager to be a HashTable.
// Pipeline constructor
// Returns the id of the created pipeline
int createPipeline(bool hasEBO, const char *vertex_shader, const char *fragment_shader);
void InitializePipelineManager();
Pipeline *GetPipeline(int id);
void FreePipelineManager();


// Renderer Constructors
void createDeferredRenderer(DeferredRenderInfo *renderInfo, bool hasEBO,
                            unsigned int width, unsigned int height);

#endif // RENDERER_H