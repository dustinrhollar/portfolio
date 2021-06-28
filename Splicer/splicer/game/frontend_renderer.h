#ifndef SPLICER_GAME_FRONTEND_RENDERER_H
#define SPLICER_GAME_FRONTEND_RENDERER_H

// defined in: frontend_renderer.cpp
extern bool FrontEndRendererNeedsResized;

struct Uniform {
    VkDeviceSize          BufferSize;
    VkBufferUsageFlags    Usage;
    VkMemoryPropertyFlags Properties; // are these three things necessary?
    
    // Buffers are persistently mapped?
    BufferParameters  Buffer;
    VmaAllocationInfo AllocInfo; // same length as Buffers, used for getting mapped memory
};

struct ResourceBindings {
    
    // NOTE(Dustin): Unused right now...
    bool MaterialBound;
    u32  Material;
    
    bool DiffuseBound;
    u32  DiffuseTexture;
};


struct RenderSystem : public jengine::ecs::ISystem
{
    VkRenderPass RenderPass;
    VkCommandPool CommandPool;
    
    // MSAA Render Target
    ImageParameters MsaaImage;
    
    // Framebuffer and Depth Resources
    ImageParameters             DepthResources;
    
    VkFramebuffer               *Framebuffer;
    u32                         FramebufferCount;
    
    VkCommandBuffer             *CommandBuffers;
    u32                         CommandBuffersCount;
    
    // Uniforms and Descriptors
    VkDescriptorPool      DescriptorPool;
    VkDescriptorPoolSize  DescriptorSizes[3];
    
    VkDescriptorSetLayout PerFrameDescriptorLayout;
    VkDescriptorSet       *PerFrameDescriptorSets;
    Uniform               *PerFrameUniforms;  // one per swapchain image
    
    VkDescriptorSetLayout PerObjectDescriptorLayout;
    VkDescriptorSet       *PerObjectDescriptorSets;
    Uniform               *PerObjectUniforms; // one per swapchain image
    
    VkDescriptorSetLayout PerMaterialDescriptorLayout;
    VkDescriptorSet       *PerMaterialDescriptorSets;
    
    // Tracks the ids of textures that are currently bound.
    ResourceBindings *BoundResources; // one per swapchain image
    
    // Minimum allowed alignment for dynamic uniform buffers
    size_t PerObjectUniformAlignment;
    
    float time = 0.0f;
    
    void Update(PlayerCamera camera, Mat4 projection_matrix);
};


bool InitializeFrontEndRenderer();
void ShutdownFrontEndRenderer();
void FrontEndRendererResize();

RenderComponent CreateRenderComponent(size_t vertex_count, size_t vertex_stride, void *vertices,
                                      size_t index_count, size_t index_stride, void *indices);
void DestroyRenderComponent(RenderComponent rcom);

#endif //SPLICER_GAME_FRONTEND_RENDERER_H
