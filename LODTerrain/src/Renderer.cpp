
Pipeline *GlobalPipelineManager;

//--------------------------------------------------------------------------------------//
// Texture Constructors
//--------------------------------------------------------------------------------------//
void createTexture(Texture *texture, unsigned int width, unsigned int height,
                   GLint internalformat, GLenum format, GLenum type, char *data)
{
    texture->width  = width;
    texture->height = height;
    
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id); 
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, internalformat, texture->width, texture->height, 0, 
                     format, type, data);
        
        // NOTE(Dustin): Probably should try to check if mips can be generated
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        // TODO(Dustin): Free recently created texture?
        std::cout << "Failed to load texture" << std::endl;
    }
}

void createFramebufferTexture(Texture *texture, unsigned int width, unsigned int height,
                              GLint internalformat, GLenum format, GLenum type,
                              GLenum attachment)
{
    texture->width  = width;
    texture->height = height;
    
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, internalformat, texture->width, texture->height, 0, 
                 format, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture->id, 0);
}

void createRenderbufferTexture(Texture *texture, unsigned int width, unsigned int height)
{
    texture->width  = width;
    texture->height = height;
    
    glGenRenderbuffers(1, &texture->id);
    glBindRenderbuffer(GL_RENDERBUFFER, texture->id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texture->width, texture->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, texture->id);
}

void FreeTexture(Texture *texture)
{
    if (!texture)
        return;
    
    glDeleteTextures ( 1, &texture->id);
}

//--------------------------------------------------------------------------------------//
// Pipeline Constructors
//--------------------------------------------------------------------------------------//
internal int AddPipeline(Pipeline *pipeline)
{
    /*
    if (GlobalPipelineManager->size + 1 >= GlobalPipelineManager->cap)
    {
        GlobalPipelineManager->cap *= 2;
        GlobalPipelineManager->pipes = (Pipeline *)realloc(GlobalPipelineManager->pipes,
                                                           sizeof(Pipeline) * GlobalPipelineManager->cap);
    }
    
    memcpy(&GlobalPipelineManager->pipes[GlobalPipelineManager->size], pipeline, sizeof(Pipeline));
    ++GlobalPipelineManager->size;
    */
    
    arrput(GlobalPipelineManager, *pipeline);
    
    return arrlen(GlobalPipelineManager) - 1;
}

int createPipeline(bool hasEBO, const char *vertex_shader, const char *fragment_shader)
{
    Pipeline pipeline = {};
    pipeline.geometryShader = new Shader(vertex_shader, fragment_shader);
    
    return AddPipeline(&pipeline);
}

// TODO(Dustin): Why is this being allocated on the heap?
#define DEFAULT_PIPELINE_LIST_SIZE 10
void InitializePipelineManager()
{
    GlobalPipelineManager = NULL;
    
    /*
    if (GlobalPipelineManager)
    {
        FreePipelineManager();
    }
    else
        GlobalPipelineManager = (PipelineList *)malloc(sizeof(PipelineList));
    GlobalPipelineManager->size  = 0;
    GlobalPipelineManager->cap   = DEFAULT_PIPELINE_LIST_SIZE;
    GlobalPipelineManager->pipes = (Pipeline *)malloc(sizeof(Pipeline) * GlobalPipelineManager->cap);
*/
}

Pipeline *GetPipeline(int id)
{
    return &GlobalPipelineManager[id];
}

void FreePipelineManager()
{
    /*
    if (!GlobalPipelineManager)
        return;
        
    for (int i = 0; i < GlobalPipelineManager->size; ++i)
    {
        Pipeline *pipe = &GlobalPipelineManager->pipes[i];
        
        FreeTexture(&pipe->diffuseTexture);
        FreeTexture(&pipe->normalTexture);
        FreeTexture(&pipe->albedoTexture);
        
        delete (pipe->geometryShader);
    }
    
    free(GlobalPipelineManager->pipes);
    GlobalPipelineManager->size = 0;
    GlobalPipelineManager->cap  = 0;
    
    free(GlobalPipelineManager);
    */
    
    arrfree(GlobalPipelineManager);
    
    GlobalPipelineManager = NULL;
}

//--------------------------------------------------------------------------------------//
// RenderInfo Constructors
//--------------------------------------------------------------------------------------//
void createDeferredRenderer(DeferredRenderInfo *renderInfo, bool hasEBO,
                            unsigned int width, unsigned int height)
{
    // Create the pipelines
    renderInfo->lightingShader = new Shader( "shaders/lightVertexShader.vs", 
                                            "shaders/lightFragmentShader.fs" );
    renderInfo->lightBoxShader = new Shader( "shaders/lightBoxVertexShader.vs", 
                                            "shaders/lightBoxFragmentShader.fs" );
    
    // Create framebuffer
    glGenFramebuffers( 1, &renderInfo->framebuffer );
    glBindFramebuffer( GL_FRAMEBUFFER, renderInfo->framebuffer );
    
    // Framebuffer textures
    createFramebufferTexture(&renderInfo->positionColorBuffer, width, height,
                             GL_RGB16F, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT0);
    createFramebufferTexture(&renderInfo->normalColorBuffer, width, height,
                             GL_RGB16F, GL_RGB, GL_FLOAT, GL_COLOR_ATTACHMENT1);
    createFramebufferTexture(&renderInfo->specColorBuffer, width, height,
                             GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT2);
    
    renderInfo->attachments[0] = GL_COLOR_ATTACHMENT0;
    renderInfo->attachments[1] = GL_COLOR_ATTACHMENT1;
    renderInfo->attachments[2] = GL_COLOR_ATTACHMENT2;
    glDrawBuffers(3, renderInfo->attachments);
    
    createRenderbufferTexture(&renderInfo->depthBuffer, width, height);
    
    // Check if framebuffer was a success
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    else
        std::cout << "Framebuffer complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
