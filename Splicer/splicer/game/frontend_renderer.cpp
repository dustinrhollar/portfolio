
bool FrontEndRendererNeedsResized = false;

file_global Shader *GlobalShader;

struct PerFrameDynamicUniformOffset {
    u32 CurrentOffset;
    u32 Size;
    u32 Increment;
};

u32 GetNextDynamicUniformOffset(PerFrameDynamicUniformOffset &uniform_offset) {
    u32 ret = uniform_offset.CurrentOffset;
    
    if (uniform_offset.CurrentOffset + uniform_offset.Increment > uniform_offset.Size) {
        printf("Attempting to request an dynamic uniform offset past the size of buffer! Looping back to the front!\n");
        uniform_offset.CurrentOffset = 0;
    }
    else {
        uniform_offset.CurrentOffset += uniform_offset.Increment;
    }
    
    return ret;
}

PerFrameDynamicUniformOffset GlobalModelDynamicUniformOffset = {};

struct PerFrameParameters {
    u32 ImageIndex;
};

PerFrameParameters GlobalFrameInfo = {};

file_internal void CreateRenderSystem(RenderSystem *rs) {
    u32 swapchain_image_count = vk::GetSwapChainImageCount();
    VkExtent2D extent = vk::GetSwapChainExtent();
    VkFormat swapchain_image_format = vk::GetSwapChainImageFormat();
    
    // Create the RenderPass
    //---------------------------------------------------------------------------
    {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format         = swapchain_image_format;
        colorAttachment.samples        = vk::GetMaxMsaaSamples();
        colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format         = vk::FindDepthFormat();
        depthAttachment.samples        = vk::GetMaxMsaaSamples();
        depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        
        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        
        VkAttachmentDescription colorAttachmentResolve = {};
        colorAttachmentResolve.format         = swapchain_image_format;
        colorAttachmentResolve.samples        = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        VkAttachmentReference colorAttachmentResolveRef = {};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkSubpassDescription subpass    = {};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpass.pResolveAttachments     = &colorAttachmentResolveRef;
        
        
        VkSubpassDependency dependency = {};
        dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass    = 0;
        dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        
        VkAttachmentDescription attachments[3] = {
            colorAttachment,
            depthAttachment,
            colorAttachmentResolve
        };
        
        rs->RenderPass = vk::CreateRenderPass(attachments, 3,
                                              &subpass, 1,
                                              &dependency, 1);
        
    }
    
    // Create the CommandPool
    {
        VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        
        rs->CommandPool = vk::CreateCommandPool(flags);
    }
    
    // Create the MSAA Render Target
    {
        VkFormat color_format = vk::GetSwapChainImageFormat();
        
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType     = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width  = (u32)(extent.width);
        imageInfo.extent.height = (u32)(extent.height);
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = color_format;
        imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage         = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples       = vk::GetMaxMsaaSamples();
        imageInfo.flags         = 0; // Optional
        
        
        VmaAllocationCreateInfo vma_create_info = {};
        vma_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        
        vk::CreateVmaImage(imageInfo,
                           vma_create_info,
                           rs->MsaaImage.Handle,
                           rs->MsaaImage.Memory,
                           rs->MsaaImage.AllocationInfo);
        
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image                           = rs->MsaaImage.Handle;
        viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format                          = color_format;
        viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;
        
        rs->MsaaImage.View = vk::CreateImageView(viewInfo);
    }
    
    // Create the Framebuffer and Depth Resources
    {
        VkFormat depth_format = vk::FindDepthFormat();
        if (depth_format == VK_FORMAT_UNDEFINED)
        {
            printf("Failed to find supported format!\n");
            return;
        }
        
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType     = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width  = (u32)(extent.width);
        imageInfo.extent.height = (u32)(extent.height);
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = depth_format;
        imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples       = vk::GetMaxMsaaSamples();
        imageInfo.flags         = 0; // Optional
        
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        
        vk::CreateVmaImage(imageInfo,
                           alloc_info,
                           rs->DepthResources.Handle,
                           rs->DepthResources.Memory,
                           rs->DepthResources.AllocationInfo);
        
        if (rs->DepthResources.Handle == VK_NULL_HANDLE) {
            printf("Error creating depth image!\n");
            return;
        }
        
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image                           = rs->DepthResources.Handle;
        viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format                          = depth_format;
        viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;
        
        rs->DepthResources.View =  vk::CreateImageView(viewInfo);
        
        vk::TransitionImageLayout(rs->CommandPool,
                                  rs->DepthResources.Handle, depth_format,
                                  VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
        
        rs->FramebufferCount = swapchain_image_count;
        rs->Framebuffer = palloc<VkFramebuffer>(swapchain_image_count);
        for (u32 i = 0; i < swapchain_image_count; ++i) {
            VkImageView attachments[] = {
                rs->MsaaImage.View,
                rs->DepthResources.View,
            };
            
            VkFramebuffer framebuffer = vk::CreateFramebuffer(attachments, 2, i, rs->RenderPass);
            rs->Framebuffer[i] = framebuffer;
        }
    }
    
    // Create Command buffers
    {
        rs->CommandBuffers = palloc<VkCommandBuffer>(swapchain_image_count);
        
        rs->CommandBuffersCount = swapchain_image_count;
        vk::CreateCommandBuffers(rs->CommandPool,
                                 VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                 swapchain_image_count,
                                 rs->CommandBuffers);
    }
    
    { // Determine alignment for the Dynamic Uniform Buffer
        // Each element in the UBO is a mat4
        size_t dyn_size = sizeof(Mat4);
        size_t req_alignment = vk::GetMinUniformMemoryOffsetAlignment();
        if (req_alignment > 0) {
            rs->PerObjectUniformAlignment =
                (req_alignment + dyn_size - 1) & ~(req_alignment - 1);
        }
        
    }
    
    rs->PerFrameUniforms  = palloc<Uniform>(swapchain_image_count); // static uniform
    rs->PerObjectUniforms = palloc<Uniform>(swapchain_image_count);  // dynamic uniform
    { // Create the Uniform Buffers
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        
        for (u32 i = 0; i < swapchain_image_count; ++i) {
            
            rs->PerFrameUniforms[i].BufferSize = sizeof(Mat4) * 2;
            rs->PerFrameUniforms[i].Usage      = usage;
            rs->PerFrameUniforms[i].Properties = properties;
            
            VkBufferCreateInfo create_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
            create_info.size  = rs->PerFrameUniforms[i].BufferSize;
            create_info.usage = rs->PerFrameUniforms[i].Usage;
            
            // Allow for the buffers to persistently mapped.
            // Can be accessed using alloc_info.pMappedData
            VmaAllocationCreateInfo alloc_info = {};
            alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
            
            vk::CreateVmaBuffer(create_info,
                                alloc_info,
                                rs->PerFrameUniforms[i].Buffer.Handle,
                                rs->PerFrameUniforms[i].Buffer.Memory,
                                rs->PerFrameUniforms[i].AllocInfo);
            
            // TODO(Dustin): Dynamic Buffer
            rs->PerObjectUniforms[i].BufferSize = rs->PerObjectUniformAlignment * 20; // 20 primitive
            rs->PerObjectUniforms[i].Usage      = usage;
            rs->PerObjectUniforms[i].Properties = properties;
            
            create_info.size = rs->PerObjectUniforms[i].BufferSize;
            
            vk::CreateVmaBuffer(create_info,
                                alloc_info,
                                rs->PerObjectUniforms[i].Buffer.Handle,
                                rs->PerObjectUniforms[i].Buffer.Memory,
                                rs->PerObjectUniforms[i].AllocInfo);
        }
    }
    
    { // Create Descriptor Information
        VkDescriptorSetLayoutBinding bindings[1]= {};
        bindings[0].binding            = 0;
        bindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[0].descriptorCount    = 1;
        bindings[0].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
        bindings[0].pImmutableSamplers = nullptr; // Optional
        
        rs->PerFrameDescriptorLayout   = vk::CreateDescriptorSetLayout(bindings, 1);
        
        bindings[0].binding            = 0;
        bindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        bindings[0].descriptorCount    = 1;
        bindings[0].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
        bindings[0].pImmutableSamplers = nullptr; // Optional
        
        rs->PerObjectDescriptorLayout  = vk::CreateDescriptorSetLayout(bindings, 1);
        
        bindings[0].binding            = 0;
        bindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[0].descriptorCount    = 1;
        bindings[0].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[0].pImmutableSamplers = nullptr; // Optional
        
        rs->PerMaterialDescriptorLayout = vk::CreateDescriptorSetLayout(bindings, 1);
        
        rs->DescriptorSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        rs->DescriptorSizes[0].descriptorCount = swapchain_image_count;
        
        rs->DescriptorSizes[1].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        rs->DescriptorSizes[1].descriptorCount = swapchain_image_count;
        
        rs->DescriptorSizes[2].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        rs->DescriptorSizes[2].descriptorCount = swapchain_image_count;
        
        // size, size_counts, max number of descritpro sets that can be allocated from this pool
        // there are two descriptor sets, each set gets one set per swapchain image
        u32 max_sets = 3 * swapchain_image_count;
        rs->DescriptorPool = vk::CreateDescriptorPool(rs->DescriptorSizes, 3, max_sets);
    }
    
    rs->PerFrameDescriptorSets    = palloc<VkDescriptorSet>(swapchain_image_count);
    rs->PerObjectDescriptorSets   = palloc<VkDescriptorSet>(swapchain_image_count);
    rs->PerMaterialDescriptorSets = palloc<VkDescriptorSet>(swapchain_image_count);
    { // Create the descriptor sets
        VkDescriptorSetLayout *layouts = talloc<VkDescriptorSetLayout>(swapchain_image_count);
        
        // Create the PerFrameDescriptorSet
        for (u32 i = 0; i < swapchain_image_count; ++i) {
            layouts[i] = rs->PerFrameDescriptorLayout;
        }
        
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool     = rs->DescriptorPool;
        allocInfo.descriptorSetCount = swapchain_image_count;
        allocInfo.pSetLayouts        = layouts;
        
        vk::CreateDescriptorSets(rs->PerFrameDescriptorSets, allocInfo);
        
        // Create the PerObjectDescriptorSet
        for (u32 i = 0; i < swapchain_image_count; ++i) {
            layouts[i] = rs->PerObjectDescriptorLayout;
        }
        
        allocInfo.pSetLayouts = layouts;
        
        vk::CreateDescriptorSets(rs->PerObjectDescriptorSets, allocInfo);
        
        // Create the PerMaterialDescriptorSet
        for (u32 i = 0; i < swapchain_image_count; ++i) {
            layouts[i] = rs->PerMaterialDescriptorLayout;
        }
        
        
        allocInfo.pSetLayouts = layouts;
        
        vk::CreateDescriptorSets(rs->PerMaterialDescriptorSets, allocInfo);
        
        // Point the DescriptorSets to the uniforms
        for (u32 i = 0; i < swapchain_image_count; ++i) {
            VkWriteDescriptorSet descriptorWrites[3] = {};
            
            // Set 0: View-Projection Matrices
            VkDescriptorBufferInfo frame_buffer_info = {};
            frame_buffer_info.buffer                 = rs->PerFrameUniforms[i].Buffer.Handle;
            frame_buffer_info.offset                 = 0;
            frame_buffer_info.range                  = VK_WHOLE_SIZE;
            
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet           = rs->PerFrameDescriptorSets[i];
            descriptorWrites[0].dstBinding       = 0;
            descriptorWrites[0].dstArrayElement  = 0;
            descriptorWrites[0].descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount  = 1;
            descriptorWrites[0].pBufferInfo      = &frame_buffer_info;
            descriptorWrites[0].pImageInfo       = nullptr; // Optional
            descriptorWrites[0].pTexelBufferView = nullptr; // Optional
            
            // Set 1: Model Matrices
            VkDescriptorBufferInfo object_buffer_info = {};
            object_buffer_info.buffer                 = rs->PerObjectUniforms[i].Buffer.Handle;
            object_buffer_info.offset                 = 0;
            object_buffer_info.range                  = VK_WHOLE_SIZE;
            
            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet           = rs->PerObjectDescriptorSets[i];
            descriptorWrites[1].dstBinding       = 0;
            descriptorWrites[1].dstArrayElement  = 0;
            descriptorWrites[1].descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            descriptorWrites[1].descriptorCount  = 1;
            descriptorWrites[1].pBufferInfo      = &object_buffer_info;
            descriptorWrites[1].pImageInfo       = nullptr; // Optional
            descriptorWrites[1].pTexelBufferView = nullptr; // Optional
            
            vk::UpdateDescriptorSets(descriptorWrites, 2);
        }
    }
    
    // Prepare the Per-Frame Resource Bindings
    rs->BoundResources = palloc<ResourceBindings>(swapchain_image_count);
    for (u32 i = 0; i < swapchain_image_count; ++i) {
        rs->BoundResources[i].DiffuseBound = false;
    }
}

bool InitializeFrontEndRenderer()
{
    if (!vk::InitializeVulkan())
        return false;
    
    RenderSystem rs = {};
    CreateRenderSystem(&rs);
    
    // TODO(Dusin): Create the Main Graphics Pipeline
    {
        //const char vshad[] = "shader.vert.spv";
        //const char fshad[] = "shader.frag.spv";
        
        const char vshad[] = "shader_animation.vert.spv";
        const char fshad[] = "shader_texture.frag.spv";
        
        VkDescriptorSetLayout layouts[3] = {
            rs.PerFrameDescriptorLayout,
            rs.PerObjectDescriptorLayout,
            rs.PerMaterialDescriptorLayout
        };
        
        void *ptr = palloc<AnimationShader>(1);
        GlobalShader = new (ptr) AnimationShader(tstring(vshad),
                                                 tstring(fshad),
                                                 rs.RenderPass,
                                                 layouts,
                                                 3);
    }
    
    InitializeTextureManager();
    InitializeMaterialManager();
    InitializeModelManager(_16MB);
    
    jengine::ecs::RegisterSystem<RenderSystem>(&rs);
    
    RenderSystem *rsys = jengine::ecs::GetSystem<RenderSystem>();
    
    return true;
}

file_internal void ShutdownRenderSystem(RenderSystem *rsys) {
    printf("SHUTTING DOWN RENDER SYSTEM\n");
    printf("----------------------------------------\n");
    // Destroy Uniform Buffers
    for (u32 i = 0; i < rsys->FramebufferCount; ++i) {
        vk::DestroyVmaBuffer(rsys->PerFrameUniforms[i].Buffer.Handle, rsys->PerFrameUniforms[i].Buffer.Memory);
        vk::DestroyVmaBuffer(rsys->PerObjectUniforms[i].Buffer.Handle, rsys->PerObjectUniforms[i].Buffer.Memory);
    }
    pfree(rsys->PerFrameUniforms);
    pfree(rsys->PerObjectUniforms);
    
    // Destroy Descriptor Set Layouts
    vk::DestroyDescriptorSetLayout(rsys->PerFrameDescriptorLayout);
    vk::DestroyDescriptorSetLayout(rsys->PerObjectDescriptorLayout);
    vk::DestroyDescriptorSetLayout(rsys->PerMaterialDescriptorLayout);
    
    // Destroy Descriptor Sets
    vk::ResetDescriptorPool(rsys->DescriptorPool);
    
    pfree(rsys->PerFrameDescriptorSets);
    pfree(rsys->PerObjectDescriptorSets);
    pfree(rsys->PerMaterialDescriptorSets);
    
    // Destroy Descriptor Pool
    vk::DestroyDescriptorPool(rsys->DescriptorPool);
    
    // Destroy Per-Frame Bindings
    pfree(rsys->BoundResources);
    
    vk::DestroyCommandBuffers(rsys->CommandPool, rsys->CommandBuffersCount,
                              rsys->CommandBuffers);
    jengine::mm::jfree(rsys->CommandBuffers);
    
    // Destroy framebuffer
    for (u32 i = 0; i < rsys->FramebufferCount; ++i)
    {
        vk::DestroyFramebuffer(rsys->Framebuffer[i]);
    }
    jengine::mm::jfree(rsys->Framebuffer);
    
    // Destroy MSAA Resource
    vk::DestroyImageView(rsys->MsaaImage.View);
    vk::DestroyVmaImage(rsys->MsaaImage.Handle, rsys->MsaaImage.Memory);
    
    // Destroy depth resources
    vk::DestroyImageView(rsys->DepthResources.View);
    vk::DestroyVmaImage(rsys->DepthResources.Handle, rsys->DepthResources.Memory);
    
    vk::DestroyCommandPool(rsys->CommandPool);
    vk::DestroyRenderPass(rsys->RenderPass);
}

void ShutdownFrontEndRenderer()
{
    // Need to wait for the last rendered frane to finish
    // before releasing objects
    vk::Idle();
    
    ShutdownModelManager();
    ShutdownMaterialManager();
    ShutdownTextureManager();
    
    RenderSystem *rsys = jengine::ecs::GetSystem<RenderSystem>();
    
    // Free the command buffers
    ShutdownRenderSystem(rsys);
    
    GlobalShader->Reset();
    pfree(GlobalShader);
    
    vk::ShutdownVulkan();
}

file_internal void RenderMesh(Mesh *mesh, Mat4 model_matrix)
{
    RenderSystem *rsys = jengine::ecs::GetSystem<RenderSystem>();
    
    // Get the next available offset into uniform memory
    u32 offset = GetNextDynamicUniformOffset(GlobalModelDynamicUniformOffset);
    
    // Copy the model matrix over
    {
        Uniform ubo = rsys->PerObjectUniforms[GlobalFrameInfo.ImageIndex];
        
        Mat4 *uniform_model = (Mat4*)((char*)ubo.AllocInfo.pMappedData + offset);
        memcpy(uniform_model, &model_matrix, sizeof(Mat4));
    }
    
    
    // Bind the descriptor with its offset
    VkCommandBuffer command_buffer = rsys->CommandBuffers[GlobalFrameInfo.ImageIndex];
    VkDescriptorSet object_ds = rsys->PerObjectDescriptorSets[GlobalFrameInfo.ImageIndex];
    vk::BindDescriptorSets(command_buffer, GlobalShader->PipelineLayout,
                           1, // set = 1
                           1, // one set being bound
                           &object_ds,
                           1, &offset);
    
    
    for (int i = 0; i < mesh->PrimitivesCount; ++i)
    {
        // NOTE(Dustin): So this is a temporary solution for per-material binding.
        // Bind object Material
        {
            MaterialParameters mat = GetMaterialComponent(mesh->Primitives[i].MaterialId).Material;
            TextureParameters tex_params;
            bool found_diffuse = false;
            if (mat.HasPBRMetallicRoughness) {
                tex_params = mat.BaseColorTexture;
                found_diffuse = true;
            }
            
            else if (mat.HasPBRSpecularGlossiness) {
                tex_params = mat.DiffuseTexture;
                found_diffuse = true;
            }
            
            else if (mat.HasClearCoat) {
                tex_params = mat.ClearCoatTexture;
                found_diffuse = true;
            }
            
            if (found_diffuse && IsValidTexture(tex_params.TextureId)) {
                bool need_descriptor_write_update = false;
                
                // If a diffuse texture is not already bound, or the current binding is out of
                // date.
                if (!rsys->BoundResources[GlobalFrameInfo.ImageIndex].DiffuseBound ||
                    (rsys->BoundResources[GlobalFrameInfo.ImageIndex].DiffuseTexture != tex_params.TextureId)) {
                    
                    rsys->BoundResources[GlobalFrameInfo.ImageIndex].DiffuseBound = true;
                    rsys->BoundResources[GlobalFrameInfo.ImageIndex].DiffuseTexture = tex_params.TextureId;
                    
                    need_descriptor_write_update = true;
                }
                
                // NOTE(Dustin): One day, we will check the other texture bindings...
                // But today is not that day.
                
                // Update the descriptor writes...
                if (need_descriptor_write_update) {
                    TextureComponent tex_component = GetTextureComponent(tex_params.TextureId);
                    
                    //for (int i = 0; i < swapchain_image_count; ++i)
                    {
                        VkWriteDescriptorSet descriptorWrites[1] = {};
                        
                        // Set 3: Material Information
                        VkDescriptorImageInfo image_info = {};
                        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        image_info.imageView   = tex_component.Texture.View;
                        image_info.sampler     = tex_component.Texture.Sampler;
                        
                        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                        descriptorWrites[0].dstSet           = rsys->PerMaterialDescriptorSets[GlobalFrameInfo.ImageIndex];
                        descriptorWrites[0].dstBinding       = 0;
                        descriptorWrites[0].dstArrayElement  = 0;
                        descriptorWrites[0].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        descriptorWrites[0].descriptorCount  = 1;
                        descriptorWrites[0].pBufferInfo      = nullptr;
                        descriptorWrites[0].pImageInfo       = &image_info; // Optiona l
                        descriptorWrites[0].pTexelBufferView = nullptr; // Optional
                        
                        vk::UpdateDescriptorSets(descriptorWrites, 1);
                    }
                }
                
                vk::BindDescriptorSets(command_buffer, GlobalShader->PipelineLayout,
                                       2, // set = 2
                                       1, // one set being bound
                                       &rsys->PerMaterialDescriptorSets[GlobalFrameInfo.ImageIndex],
                                       0, nullptr);
            }
        }
        
        RenderComponent rcomp = mesh->Primitives[i].RenderComp;
        
        VkBuffer vertexBuffers[] = {rcomp.VertexBuffer.Handle};
        VkDeviceSize offsets[] = {0};
        
        vk::BindVertexBuffers(command_buffer, 0, 1, vertexBuffers, offsets);
        
        if (rcomp.IndexedDraw) {
            vk::BindIndexBuffer(command_buffer, rcomp.IndexBuffer.Handle, 0, VK_INDEX_TYPE_UINT32);
            
            vk::DrawIndexed(command_buffer, rcomp.tri_count,
                            1, 0, 0, 0);
        }
        else {
            vk::Draw(command_buffer, rcomp.tri_count, 1, 0, 0);
        }
    }
}

file_internal void RenderNode(MeshNode *node, Mat4 matrix)
{
    Mat4 node_matrix = Mat4(1.0f);
    
    if (node->HasMatrix)
    {
        node_matrix = node->Matrix;
    }
    else
    {
        Mat4 translation_matrix = Mat4(1.0f);
        Mat4 scale_matrix = Mat4(1.0f);
        Mat4 rotation_matrix = Mat4(1.0f);
        
        if (node->HasTranslation)
        {
            translation_matrix = Translate(node->Translation);
            
        }
        
        if (node->HasScale)
        {
            scale_matrix = Scale(node->Scale.x, node->Scale.y, node->Scale.z);
        }
        
        if (node->HasRotation)
        {
            Vec3 axis = node->Rotation.xyz;
            float theta = node->Rotation.w;
            
            Quaternion rotation = MakeQuaternion(axis.x,axis.y,axis.z,theta);
            rotation_matrix = GetQuaternionRotationMatrix(rotation);
        }
        
        // Multiplication order = T * R * S
        node_matrix = Mul(node_matrix, scale_matrix);
        node_matrix = Mul(node_matrix, rotation_matrix);
        node_matrix = Mul(node_matrix, translation_matrix);
    }
    
    Mat4 model_matrix = Mul(matrix, node_matrix);
    
    if (node->NodeMesh)
    {
        RenderMesh(node->NodeMesh, model_matrix);
    }
    
    for (int i = 0; i < node->ChildrenCount; ++i)
    {
        RenderNode(node->Children + i, model_matrix);
    }
}

void RenderSystem::Update(PlayerCamera camera, Mat4 projection_matrix)
{
    { // Update resources on the GPU (if necessary)
        UploadBufferedTextures(CommandPool);
    }
    
    // Begin the frame
    u32 image_index;
    
    VkResult khr_result = vk::BeginFrame(image_index);
    if (khr_result == VK_ERROR_OUT_OF_DATE_KHR) {
        FrontEndRendererResize();
        return;
    } else if (khr_result != VK_SUCCESS &&
               khr_result != VK_SUBOPTIMAL_KHR) {
        printf("Failed to acquire swap chain image!");
    }
    
    VkCommandBuffer command_buffer = CommandBuffers[image_index];
    VkFramebuffer framebuffer = Framebuffer[image_index];
    
    GlobalFrameInfo.ImageIndex = image_index;
    // Begin CommandBuffer recording and start the renderpass
    {
        vk::BeginCommandBuffer(command_buffer);
        
        VkExtent2D extent = vk::GetSwapChainExtent();
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width  = (float) extent.width;
        viewport.height = (float) extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        
        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = extent;
        
        vk::SetViewport(command_buffer, 0, 1, &viewport);
        vk::SetScissor(command_buffer, 0, 1, &scissor);
        
        VkClearValue clear_values[2] = {};
        
        // clear back ground
        clear_values[0].color = {0.67f, 0.85f, 0.90f, 1.0f};
        // clear depth
        clear_values[1].depthStencil = {1.0f, 0};
        
        vk::BeginRenderPass(command_buffer, clear_values, 2, framebuffer, RenderPass);
    }
    
    // Drawing commands
    {
        // Setup the dynamic uniforms for this frame
        GlobalModelDynamicUniformOffset.CurrentOffset = 0;
        GlobalModelDynamicUniformOffset.Size = (u32)PerObjectUniforms[image_index].BufferSize;
        GlobalModelDynamicUniformOffset.Increment = (u32)PerObjectUniformAlignment;
        
        // Copy Per-Frame Descriptors into Uniform memory
        {
            Uniform *frame_uniform = &PerFrameUniforms[image_index];
            void *uniform_ptr = (frame_uniform->AllocInfo.pMappedData);
            
            Mat4 view = camera.GetViewMatrix();
            
            struct VP {
                Mat4 View;
                Mat4 Proj;
            } vp;
            
            vp.View = view;
            vp.Proj = projection_matrix;
            vp.Proj[1][1] *= -1;
            
            memcpy(uniform_ptr, &vp, sizeof(VP));
        }
        
        // Descriptor Sets for this frame
        VkDescriptorSet frame_ds = PerFrameDescriptorSets[image_index];
        VkDescriptorSet object_ds = PerObjectDescriptorSets[image_index];
        
        // Bind the global descriptors (View + Projection Matrices)
        vk::BindDescriptorSets(command_buffer, GlobalShader->PipelineLayout,
                               0, 1,
                               &frame_ds,
                               0, 0);
        
        // Bind the GlobalShader (there is only one for now)
        GlobalShader->Bind(command_buffer);
        
        // Draw any models in the scene
        for (u32 i = 0; i < GlobalCurrentModelCount; ++i)
        {
            Model model = ModelRegistry[i];
            
            for (u32 j = 0; j < model.nodes_count; ++j)
            {
                Mat4 model_matrix = Mat4(1.0f);
                
                MeshNode *node = model.nodes + j;
                RenderNode(node, model_matrix);
            }
        }
        
    }
    
    // End the frame
    {
        vk::EndRenderPass(command_buffer);
        vk::EndCommandBuffer(command_buffer);
        vk::EndFrame(image_index, command_buffer);
        
        if (khr_result == VK_ERROR_OUT_OF_DATE_KHR ||
            khr_result == VK_SUBOPTIMAL_KHR ||
            FrontEndRendererNeedsResized) {
            
            FrontEndRendererResize();
            FrontEndRendererNeedsResized = false;
        }
        else if (khr_result != VK_SUCCESS) {
            printf("Something went wrong acquiring the swapchain image!\n");
        }
    }
}

RenderComponent CreateRenderComponent(size_t vertex_count, size_t vertex_stride, void *vertices,
                                      size_t index_count, size_t index_stride, void *indices)
{
    
    RenderSystem *rsys = jengine::ecs::GetSystem<RenderSystem>();
    
    RenderComponent rcomp = {};
    
    // Create vertex info
    VkBufferCreateInfo vertex_buffer_info = {};
    vertex_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertex_buffer_info.size = vertex_count * vertex_stride;
    vertex_buffer_info.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    
    VmaAllocationCreateInfo vertex_alloc_info = {};
    vertex_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    
    
    vk::CreateVmaBufferWithStaging(rsys->CommandPool,
                                   vertex_buffer_info,
                                   vertex_alloc_info,
                                   rcomp.VertexBuffer.Handle,
                                   rcomp.VertexBuffer.Memory,
                                   vertices,
                                   vertex_count * vertex_stride);
    
    rcomp.VertexBuffer.Size = vertex_count * vertex_stride;
    
    if (index_count > 0)
    {
        rcomp.IndexedDraw = true;
        // Create index info
        VkBufferCreateInfo index_buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        index_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        index_buffer_info.size = index_count * index_stride;
        index_buffer_info.usage =
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        
        VmaAllocationCreateInfo index_alloc_info = {};
        index_alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        
        vk::CreateVmaBufferWithStaging(rsys->CommandPool,
                                       index_buffer_info,
                                       index_alloc_info,
                                       rcomp.IndexBuffer.Handle,
                                       rcomp.IndexBuffer.Memory,
                                       indices,
                                       index_count * index_stride);
        
        rcomp.IndexBuffer.Size = index_count * index_stride;
        
        rcomp.tri_count = (u32)index_count;
    }
    else
    {
        rcomp.IndexedDraw = false;
        rcomp.tri_count = (u32)vertex_count;
    }
    
    return rcomp;
};

void DestroyRenderComponent(RenderComponent rcom) {
    vk::DestroyVmaBuffer(rcom.VertexBuffer.Handle, rcom.VertexBuffer.Memory);
    vk::DestroyVmaBuffer(rcom.IndexBuffer.Handle, rcom.IndexBuffer.Memory);
}

void FrontEndRendererResize() {
    
    // Platform.WaitForEvents() <- wait if the app has been minimized
    Platform.WaitForEvents();
    
    // Idle <- wait for last frame to finish rendering
    vk::Idle();
    
    // Destroy RenderPass, Framebuffers, DepthResources, Commandbuffers
    RenderSystem *rsys = jengine::ecs::GetSystem<RenderSystem>();
    
    // Free the command buffers
    // NOTE(Dustin): This only works because RenderSystem State is dependent
    // on the swapchain. This will no longer work when RS contains state that
    // is not dependent on the swapchain
    ShutdownRenderSystem(rsys);
    
    vk:: Resize();
    
    // Recreate the RenderSystem
    CreateRenderSystem(rsys);
    // -- Destroy swapchain
    // -- Create swapchain
}
