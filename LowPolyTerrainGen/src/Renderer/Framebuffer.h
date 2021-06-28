#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

typedef enum 
{
    // Color attachments
    // - Guarenteed to have at least 8 attachments
    Attachment_Color0, 
    Attachment_Color1, 
    Attachment_Color2, 
    Attachment_Color3, 
    Attachment_Color4, 
    Attachment_Color5, 
    Attachment_Color6, 
    Attachment_Color7,

    Attachment_DepthTexture,
    Attachment_Depth,
    Attachment_Stencil,
    Attachment_DepthStencil,

    Attachment_Count,
    Attachment_Unknown = Attachment_Count,
} ETextureAttachment;


typedef struct
{
    u32 handle;
    u32 width;
    u32 height;
    // A bitfield where all 1s are attached textures
    u16 bitfield;
    // Texture Ids for the framebuffer attachments
    u32 attachments[Attachment_Count];
} Framebuffer;

void fb_init(Framebuffer *fb, u32 width, u32 height);
void fb_free(Framebuffer *fb);
void fb_bind(Framebuffer *fb);
void fb_unbind(Framebuffer *fb, u32 restore_width, u32 restore_height);
// Creates a Texture Attachment for a *already bound framebuffer*
void fb_attach_texture(Framebuffer *fb, ETextureAttachment attachment);
u32 fb_get_texture(Framebuffer *fb, ETextureAttachment attachment);

#endif//_FRAMEBUFFER_H
