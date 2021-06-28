
static GLenum texture_attachment_to_glenum(ETextureAttachment a)
{
    GLenum result = GL_COLOR_ATTACHMENT0;

    if (a == Attachment_Color0)
    {
        result = GL_COLOR_ATTACHMENT0;
    }
    else if (a < Attachment_DepthTexture)
    {
        // NOTE(Dustin): Okay, this might not actually work, so
        // be aware future me that will one day hate me.
        result += (a - Attachment_Color0);
    }
    else if (a == Attachment_DepthTexture || a == Attachment_Depth)
    {
        result = GL_DEPTH_ATTACHMENT;
    }
    else if (a == Attachment_Stencil)
    {
        result = GL_STENCIL_ATTACHMENT;
    }
    else if (a == Attachment_DepthStencil)
    {
        result = GL_DEPTH_STENCIL_ATTACHMENT;
    }

    return result;
}

void fb_init(Framebuffer *fb, u32 width, u32 height)
{
    fb->width  = width;
    fb->height = height;
    fb->bitfield = 0;
    glGenFramebuffers(1, &fb->handle);
    glBindFramebuffer(GL_FRAMEBUFFER, fb->handle);
    // indicate that will draw to color0
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

void fb_free(Framebuffer *fb)
{
    while (fb->bitfield > 0)
    {
        u32 index = PlatformClz(fb->bitfield);

        if (index < Attachment_Depth)
        {
            glDeleteTextures(1, &fb->attachments[index]);
        }
        else if (index == Attachment_Depth)
        {
            glDeleteRenderbuffers(1, &fb->attachments[index]);
        }
        else if (index == Attachment_Stencil)
        {
            LogWarn("Framebuffer Free Warning: Attempting to free unsupported Texture Attachment (Stencil)");
        }
        else if (index == Attachment_DepthStencil)
        {
            LogWarn("Framebuffer Free Warning: Attempting to free unsupported Texture Attachment (DepthStencil)");
        }
        else
        {
            LogError("Found bad index when clearing Framebuffer bitfield: %d!", index);
        }

        BIT32_TOGGLE_0(fb->bitfield, index);
    }

    glDeleteFramebuffers(1, &fb->handle);
}

void fb_bind(Framebuffer *fb)
{
    glBindTexture(GL_TEXTURE_2D, 0); // make sure the texture is not bound
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb->handle);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glViewport(0, 0, fb->width, fb->height);
}

void fb_unbind(Framebuffer *fb, u32 restore_width, u32 restore_height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // NOTE(Dustin): Should this be the caller's job?
    glViewport(0, 0, restore_width, restore_height);
}

// Creates a Texture Attachment for a *already bound framebuffer*
void fb_attach_texture(Framebuffer *fb, ETextureAttachment attachment)
{
    GLenum slot = texture_attachment_to_glenum(attachment);
    u32 texture;

    if (attachment < Attachment_DepthTexture)
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fb->width, fb->height,
				0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, slot, GL_TEXTURE_2D, texture, 0);
    }
    else if (attachment == Attachment_DepthTexture)
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, fb->width, fb->height,
				0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
    }
    else if (attachment == Attachment_Depth)
    {
        glGenRenderbuffers(1, &texture);
		glBindRenderbuffer(GL_RENDERBUFFER, texture);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fb->width, fb->height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, slot, GL_RENDERBUFFER, texture);
    }
    else if (attachment == Attachment_Stencil)
    {
        LogWarn("Attempted to create stencil attachment for framebuffer. This is not supported right now.");
    }
    else if (attachment == Attachment_DepthStencil)
    {
        LogWarn("Attempted to create depth-stencil attachment for framebuffer. This is not supported right now.");
    }
    else
    {
        LogError("Unknown texture attachment: %d", attachment);
    }
        
    fb->attachments[attachment] = texture;
    BIT32_TOGGLE_1(fb->bitfield, attachment);
}

u32 fb_get_texture(Framebuffer *fb, ETextureAttachment attachment)
{
    u32 result = fb->attachments[attachment];
    return result;
}
