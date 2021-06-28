#ifndef SPLICER_GAME_MATERIAL_H
#define SPLICER_GAME_MATERIAL_H

/*

From the GLTF 2.0 Specifications:

The alphaMode property defines how the alpha value of the main factor and texture should be
interpreted. The alpha value is defined in the baseColor for metallic-roughness material model.

alphaMode can be one of the following values:

    OPAQUE - The rendered output is fully opaque and any alpha value is ignored.
    MASK   - The rendered output is either fully opaque or fully transparent depending on the alpha
             value and the specified alpha cutoff value. This mode is used to simulate geometry such
             as tree leaves or wire fences.
    BLEND  - The rendered output is combined with the background using the normal painting operation
             (i.e. the Porter and Duff over operator). This mode is used to simulate geometry such
             as guaze cloth or animal fur.

When alphaMode is set to MASK the alphaCutoff property specifies the cutoff threshold. If the alpha
value is greater than or equal to the alphaCutoff value then it is rendered as fully opaque,
otherwise, it is rendered as fully transparent. alphaCutoff value is ignored for other modes.

Implementation Note:

OPAQUE - A depth value is written for every pixel and mesh sorting is not required for correct output.
MASK   - A depth value is not written for a pixel that is discarded after the alpha test. A depth
         value is written for all other pixels. Mesh sorting is not required for correct output.
BLEND  - Support for this mode varies. There is no perfect and fast solution that works for all cases.
         Implementations should try to achieve the correct blending output for as many situations as
         possible. Whether depth value is written or whether to sort is up to the implementation.
         For example, implementations can discard pixels which have zero or close to zero alpha
         value to avoid sorting issues.

*/
enum TextureAlphaMode {
    TEXTURE_ALPHA_MODE_OPAQUE,
    TEXTURE_ALPHA_MODE_MASK,
    TEXTURE_ALPHA_MODE_BLEND,
    TEXTURE_ALPHA_MODE_INVALID,
};


struct TextureParameters {
    VkFilter MagFilter; // store?
    VkFilter MinFilter; // store?
    
    VkSamplerAddressMode AddressModeU; // wrapS
    VkSamplerAddressMode AddressModeV; // wrapT
    VkSamplerAddressMode AddressModeW; // ??
    
    u32 TextureId;
    
    // cgltf contains these values, but there doesn't seem to be anything
    // in the specs that mention them
    //bool HasTransform;
    //vec3 Scale;
    //Vec2 Offset;
    //r32  Theta; // Radians or Degrees?
    
};


struct MaterialParameters
{
    // NOTE(Dustin): Name stored in a SystemInfo.
    // It is redundant to store the name here.
    jstring Name;
    
    // NOTE(Dustin): For now, I am going to assumed that only
    // one of these will ever be true - that way each pipeline's
    // settings can be placed in a union to reduce struct size/
    
    // TODO(Dustin): Assuming that only one of these can be true,
    // turn the booleans into an enumeration, or even make different
    // meterial parameter types.
    bool HasPBRMetallicRoughness;
    bool HasPBRSpecularGlossiness;
    bool HasClearCoat;
    
    union {
        // Metallic - Roughness Pipeline
        struct {
            TextureParameters BaseColorTexture;
            TextureParameters MetallicRoughnessTexture;
            
            Vec4 BaseColorFactor; // Will this always be a vec4?
            r32  MetallicFactor;
            r32  RoughnessFactor;
        };
        
        // Specilar - Glosiness Pipeline
        struct {
            TextureParameters DiffuseTexture;
            TextureParameters SpecularGlossinessTexture;
            
            Vec4 DiffuseFactor;
            Vec3 SpecularFactor;
            r32  GlossinessFactor;
        };
        
        // ClearCoat Pipeline
        struct {
            TextureParameters ClearCoatTexture;
            TextureParameters ClearCoatRoughnessTexture;
            TextureParameters ClearCoatNormalTexture;
            
            r32 ClearCoatFactor;
            r32 ClearCoarRoughnessFactor;
        };
    };
    
    TextureParameters NormalTexture;
    TextureParameters OcclusionTexture;
    TextureParameters EmissiveTexture;
    
    // Alpha mode?
    TextureAlphaMode AlphaMode;
    r32 AlphaCutoff;
    
    bool DoubleSided;
    bool Unlit;
};


struct MaterialComponent {
    VkDescriptorSetLayout *DescriptorSetLayouts;
    u32                   DescriptorSetLayoutsCount;
    
    VkDescriptorSet *DescriptorSets;
    u32             DescriptorSetsCount;
    
    // Static buffers (i.e. Textures?)
    MaterialParameters Material;
    
    
    // Dynamic buffers (i.e. Material Instances)
};

void InitializeMaterialManager();
void ShutdownMaterialManager();

u32 CreateMaterial(MaterialParameters material_param);
bool IsValidMaterial(u32 id);
jstring GetMaterialName(u32 id);
MaterialComponent GetMaterialComponent(u32 id);


#endif // SPLICER_GAME_MATERIAL_H
