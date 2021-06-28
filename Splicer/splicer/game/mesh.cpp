
using namespace jengine;

file_global void *GlobalModelFreeListMemory = nullptr;
file_global mm::FreeListAllocator *GlobalModelAllocator = nullptr;
Model ModelRegistry[MAX_MODELS_IN_REGISTRY];
u32 GlobalCurrentModelCount;

//-----------------------------------------------------------------------------------------------
// Mesh Manager
//-----------------------------------------------------------------------------------------------
file_internal void ShutdownMesh(Mesh *mesh);
file_internal void ShutdownNode(MeshNode *mode);

void InitializeModelManager(size_t size)
{
    GlobalModelFreeListMemory = palloc(size + sizeof(mm::FreeListAllocator));
    GlobalModelAllocator = new (GlobalModelFreeListMemory) mm::FreeListAllocator(size,
                                                                                 (char*)GlobalModelFreeListMemory + sizeof(mm::FreeListAllocator));
    GlobalCurrentModelCount = 0;
}

void ShutdownModelManager()
{
    for (u32 i = 0; i < GlobalCurrentModelCount; ++i)
    {
        Model model = ModelRegistry[i];
        
        for (u32 j = 0; j < model.nodes_count; ++j)
        {
            MeshNode *node = model.nodes + j;
            ShutdownNode(node);
        }
    }
    
    GlobalModelAllocator->ReleaseMemory();
    GlobalModelAllocator= nullptr;
    pfree(GlobalModelFreeListMemory);
    
    GlobalCurrentModelCount = 0;
}

file_internal void ShutdownMesh(Mesh *mesh) {
    for (int i = 0; i < mesh->PrimitivesCount; ++i)
    {
        RenderComponent rcomp = mesh->Primitives[i].RenderComp;
        
        DestroyRenderComponent(rcomp);
    }
}

file_internal void ShutdownNode(MeshNode *node) {
    
    node->Name.Clear();
    
    if (node->NodeMesh) {
        ShutdownMesh(node->NodeMesh);
    }
    
    for (int i = 0; i < node->ChildrenCount; ++i) {
        ShutdownNode(node->Children + i);
    }
}


//-----------------------------------------------------------------------------------------------
// Mesh Loader
//-----------------------------------------------------------------------------------------------
// TODO(Dustin):
// - Attributes: tangents, joints, weights, Anim Samplers
// - Log Errors
// NOTE(Dustin):
// - Current approach to index into buffer is buffer->offset + buffer_view->offet
//   this might not be right when there are multple buffers

inline TextureAlphaMode CgAlphaModeToVk(cgltf_alpha_mode alpha_mode) {
    
    switch(alpha_mode) {
        
        case cgltf_alpha_mode_opaque:
        return TEXTURE_ALPHA_MODE_OPAQUE;
        
        case cgltf_alpha_mode_mask:
        return TEXTURE_ALPHA_MODE_MASK;
        
        case cgltf_alpha_mode_blend:
        return TEXTURE_ALPHA_MODE_BLEND;
        
        default: return TEXTURE_ALPHA_MODE_INVALID;
    }
}

class MeshLoader {
    public:
    
    MeshLoader(jstring &filename);
    ~MeshLoader();
    
    void LoadMesh(Model **models, size_t *model_count);
    
    private:
    
    cgltf_data *Data;
    
    jstring Filename;
    jstring Directory;
    
    HashTable<cgltf_node*, MeshNode*> NodeMappping;
    
    void ParseAnimation(cgltf_animation* cg_anim, Animation *animation);
    void ParseSkeleton();
    void ParseTexture(cgltf_texture *texture, TextureParameters *tex_param);
    void ParseMaterial(cgltf_material *cg_material, MaterialParameters *material);
    void ParsePrimitive(cgltf_primitive *cg_primitive, Primitive *primitive);
    void ParseMesh(cgltf_mesh *cg_mesh, Mesh *mesh);
    void ParseNode(cgltf_node *cg_node, MeshNode  *node);
    void ParseScene(cgltf_scene *scene, Model *model);
};

MeshLoader::MeshLoader(jstring &filename) {
    Filename = filename;
    
    const char *cfile = Filename.GetCStr();
    
    const char* ptr = strrchr(cfile, '/');
    if (ptr == nullptr) {
        ptr = strrchr(cfile, '\\');
        
        if (ptr == nullptr) {
            printf("Could not find directory!\n");
        }
    }
    
    ptr++;
    
    size_t len = ptr - cfile;
    char *dirname = (char*)talloc(len);
    strncpy(dirname, cfile, len);
    
    Directory = jstring(dirname, (u32)len);
    
    printf("Filename %s\n", cfile);
    printf("Directory %s\n", Directory.GetCStr());
}


MeshLoader::~MeshLoader() {
    cgltf_free(Data);
}


// Use for getting the wrap_t and wrap_s
inline VkSamplerAddressMode CgIntToVkAddressMode(cgltf_int address_mode) {
    switch (address_mode) {
        // wrap_s and wrap_t
        case 33071: // CLAMP_TO_EDGE
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        
        case 33648: // MIRRORED_REPEAT
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        
        case 10497: // REPEAT
        default:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        
    }
}

// used for getting the mag and min filter
inline VkFilter CgIntToVkFilter(cgltf_int filter)
{
    switch (filter) {
        // Mag + Min Filter
        case 9728: // NEAREST
        return VK_FILTER_NEAREST;
        
        case 9729: // LINEAR
        return VK_FILTER_LINEAR;
        
        // Min Filter
        // By default, these values are not within the vulkan specs
        case 9984: // NEAREST_MIPMAP_NEAREST
        case 9985: // LINEAR_MIPMAP_NEAREST
        case 9986: // NEAREST_MIPMAP_LINEAR
        case 9987: // LINEAR_MIPMAP_LINEAR
        
        default: return VK_FILTER_NEAREST;
    }
}

void MeshLoader::ParseAnimation(cgltf_animation* cg_anim, Animation *animation)
{
    KeyFrameList key_frame_list = {};
    CreateKeyFrameList(&key_frame_list);
    
    for (int i = 0; i < cg_anim->channels_count; ++i)
    {
        KeyFrameListIterator key_frame_iterator;
        CreateKeyFrameIterator(&key_frame_list, &key_frame_iterator);
        
        cgltf_animation_channel cg_anim_channel = cg_anim->channels[i];
        cgltf_animation_sampler *cg_anim_sampler = cg_anim_channel.sampler;
        
        MeshNode *ptr = nullptr;
        if (!NodeMappping.Get(cg_anim_channel.target_node, &ptr))
        {
            printf("Failed to retrive the node pointer!\n");
        }
        
        cgltf_accessor *input  = cg_anim_sampler->input;
        cgltf_accessor *output = cg_anim_sampler->output;
        
        u64 input_count = input->count;
        u64 input_offset = input->offset;
        
        cgltf_buffer_view *input_buffer_view = input->buffer_view;
        cgltf_buffer_view *output_buffer_view = output->buffer_view;
        
        void *channel_input_data = (char*)input_buffer_view->buffer->data + input_buffer_view->offset + input_offset;
        float *time_start = (float*)channel_input_data;
        
        // Channel information is the output
        void *channel_output_data = (char*)output_buffer_view->buffer->data + output_buffer_view->offset + output->offset;
        Channel channel = {};
        channel.Joint = ptr;
        
        switch (cg_anim_channel.target_path)
        {
            case cgltf_animation_path_type_invalid:
            {
                channel.Type = CHANNEL_TYPE_INVALID;
            } break;
            
            case cgltf_animation_path_type_translation:
            {
                channel.Type = CHANNEL_TYPE_TRANSLATION;
            } break;
            
            case cgltf_animation_path_type_rotation:
            {
                channel.Type = CHANNEL_TYPE_ROTATION;
            } break;
            
            case cgltf_animation_path_type_scale:
            {
                channel.Type = CHANNEL_TYPE_SCALING;
            } break;
            
            case cgltf_animation_path_type_weights:
            {
                channel.Type = CHANNEL_TYPE_WEIGHTS;
            } break;
        }
        
        for (int j = 0; j < input_count; ++j)
        {
            switch (channel.Type)
            {
                case CHANNEL_TYPE_ROTATION:
                {
                    float *rotation = (float*)((char*)channel_output_data + j * output->stride);
                    channel.Rotation = MakeQuaternion(rotation[0],rotation[1],rotation[2],rotation[3]);
                } break;
                
                case CHANNEL_TYPE_TRANSLATION:
                case CHANNEL_TYPE_SCALING:
                {
                    float *rotation = (float*)((char*)channel_output_data + j * output->stride);
                    channel.Scaling = MakeVec3(rotation);
                } break;
                
                case CHANNEL_TYPE_WEIGHTS:
                case CHANNEL_TYPE_INVALID:
                default:
                {
                    printf("Unsupported animation channel found!\n");
                } break;
            }
            
            AddToKeyFrameList(&key_frame_iterator, time_start[j], channel);
        }
    }
    
    size_t count = key_frame_list.Size;
    KeyFrame *key_frames = palloc<KeyFrame>(count);
    CollapseKeyFrameListToArray(&key_frame_list, key_frames);
    
    animation->Name = pstring(cg_anim->name);
    animation->KeyFramesCount = count;
    // TODO(Dustin): hmmm...goes into global memory?
    animation->KeyFrames = palloc<KeyFrame>(animation->KeyFramesCount);
    CollapseKeyFrameListToArray(&key_frame_list, animation->KeyFrames);
    
#if 0
    printf("Parsing Animation Results\n");
    for (int i = 0; i < count; ++i)
    {
        printf("Key Frame %d\n", i);
        
        for (u32 j = 0; j < key_frames[i].Channels.Size(); ++j)
        {
            Channel channel = key_frames[i].Channels[j];
            printf("\tTime %p\n", key_frames[i].StartTime);
            printf("\tJoint %p\n", channel.Joint);
            switch (channel.Type)
            {
                case CHANNEL_TYPE_ROTATION:
                {
                    printf("\t\tQuaternion: %lf %lf %lf %lf\n", channel.Rotation.x,
                           channel.Rotation.y, channel.Rotation.z, channel.Rotation.w);
                } break;
                
                case CHANNEL_TYPE_TRANSLATION:
                {
                    printf("\t\tTranslation: %lf %lf %lf %lf\n", channel.Translation.x,
                           channel.Translation.y, channel.Translation.z);
                } break;
                
                case CHANNEL_TYPE_SCALING:
                {
                    printf("\t\tScaling: %lf %lf %lf %lf\n", channel.Scaling.x,
                           channel.Scaling.y, channel.Scaling.z);
                } break;
                
                case CHANNEL_TYPE_WEIGHTS:
                case CHANNEL_TYPE_INVALID:
                default:
                {
                    printf("Unsupported animation channel found!\n");
                } break;
            }
        }
        
        printf("\tChannels Count %d\n", key_frames[i].Channels.Size());
    }
    printf("\n\n");
#endif
}

void MeshLoader::ParseTexture(cgltf_texture *texture, TextureParameters *tex_param)
{
    if (texture->sampler) {
        tex_param->MagFilter = CgIntToVkFilter(texture->sampler->mag_filter); // store?
        tex_param->MinFilter = CgIntToVkFilter(texture->sampler->min_filter); // store?
        
        tex_param->AddressModeU = CgIntToVkAddressMode(texture->sampler->wrap_s); // wrapS
        tex_param->AddressModeV = CgIntToVkAddressMode(texture->sampler->wrap_t); // wrapT
        tex_param->AddressModeW = CgIntToVkAddressMode(texture->sampler->wrap_s); // ??
    }
    // Load the texture
    jstring fullpath = jstring(((u32)strlen(texture->image->uri)) + Directory.GetLen());
    AddJString(fullpath, Directory, texture->image->uri);
    tex_param->TextureId = LoadTexture(fullpath);
}

void MeshLoader::ParseMaterial(cgltf_material *cg_material, MaterialParameters *material)
{
    if (cg_material->name) {
        material->Name = jstring(cg_material->name);
        printf("Material %s.\n", material->Name.GetCStr());
    }
    
    material->HasPBRMetallicRoughness  = cg_material->has_pbr_metallic_roughness;
    material->HasPBRSpecularGlossiness = cg_material->has_pbr_specular_glossiness;
    material->HasClearCoat             = cg_material->has_clearcoat;
    
    // NOTE(Dustin): This is a very silly if-statement, but I want to be able to detect
    // if there will ever be a combination of these settings
    if (material->HasPBRMetallicRoughness  && material->HasPBRSpecularGlossiness ||
        material->HasPBRMetallicRoughness  && material->HasClearCoat ||
        material->HasPBRSpecularGlossiness && material->HasClearCoat ||
        material->HasPBRMetallicRoughness  && material->HasPBRSpecularGlossiness && material->HasClearCoat)
    {
        printf("%s Meterial has a combination of material types!\n", material->Name.GetCStr());
    }
    
    if (material->HasPBRMetallicRoughness) {
        cgltf_pbr_metallic_roughness cmr = cg_material->pbr_metallic_roughness;
        
        cgltf_texture_view bctv = cmr.base_color_texture;
        cgltf_texture_view mrtv = cmr.metallic_roughness_texture;
        
        if (bctv.texture) {
            cgltf_texture *bct = bctv.texture;
            ParseTexture(bct, &material->BaseColorTexture);
        }
        
        if (mrtv.texture) {
            cgltf_texture *mrt = mrtv.texture;
            ParseTexture(mrt, &material->MetallicRoughnessTexture);
        }
        
        Vec4 BaseColorFactor = MakeVec4(cmr.base_color_factor); // Will this always be a vec4?
        r32  MetallicFactor  = cmr.metallic_factor;
        r32  RoughnessFactor = cmr.roughness_factor;
    }
    else if (material->HasPBRSpecularGlossiness) {
        cgltf_pbr_specular_glossiness csg = cg_material->pbr_specular_glossiness;
        
        cgltf_texture_view dv = csg.diffuse_texture;
        cgltf_texture_view sgtv = csg.specular_glossiness_texture;
        
        if (dv.texture) {
            cgltf_texture *dt = dv.texture;
            ParseTexture(dt, &material->DiffuseTexture);
        }
        
        if (sgtv.texture) {
            cgltf_texture *sgt = sgtv.texture;
            ParseTexture(sgt, &material->SpecularGlossinessTexture);
        }
        
        material->DiffuseFactor    = MakeVec4(csg.diffuse_factor);
        material->SpecularFactor   = MakeVec3(csg.specular_factor);
        material->GlossinessFactor = csg.glossiness_factor;;
    }
    else if (material->HasClearCoat) {
        cgltf_clearcoat cc = cg_material->clearcoat;
        
        cgltf_texture_view cctv = cc.clearcoat_texture;
        cgltf_texture_view ccrtv = cc.clearcoat_roughness_texture;
        cgltf_texture_view ccntv = cc.clearcoat_normal_texture;
        
        if (cctv.texture) {
            cgltf_texture *cct = cctv.texture;
            ParseTexture(cct, &material->ClearCoatTexture);
        }
        
        if (ccrtv.texture) {
            cgltf_texture *ccrt = ccrtv.texture;
            ParseTexture(ccrt, &material->ClearCoatRoughnessTexture);
        }
        
        if (ccntv.texture) {
            cgltf_texture *ccnt = ccrtv.texture;
            ParseTexture(ccnt, &material->ClearCoatNormalTexture);
        }
        
        material->ClearCoatFactor = cc.clearcoat_factor;
        material->ClearCoarRoughnessFactor = cc.clearcoat_roughness_factor;
        
    }
    
    cgltf_texture_view normal_texture_view    = cg_material->normal_texture;
    cgltf_texture_view occlusion_texture_view = cg_material->occlusion_texture;
    cgltf_texture_view emissive_texture_view  = cg_material->emissive_texture;
    
    if (normal_texture_view.texture) {
        cgltf_texture *nt = normal_texture_view.texture;
        ParseTexture(nt, &material->NormalTexture);
    }
    
    if (occlusion_texture_view.texture) {
        cgltf_texture *ot = normal_texture_view.texture;
        ParseTexture(ot, &material->OcclusionTexture);
    }
    
    if (emissive_texture_view.texture) {
        cgltf_texture *et = emissive_texture_view.texture;
        ParseTexture(et, &material->EmissiveTexture);
    }
    
    material->AlphaMode   = CgAlphaModeToVk(cg_material->alpha_mode);
    material->AlphaCutoff = cg_material->alpha_cutoff;
    
    material->DoubleSided = cg_material->double_sided;
    material->Unlit       = cg_material->unlit;
}

file_internal void ParseVertex(Vertex *vertices,
                               float  *position_buffer,
                               float  *normal_buffer,
                               float  *color_buffer,
                               float  *uv0_buffer,
                               size_t count)
{
    Vec2 dummy_vec2 = { 0.0f, 0.0f };
    Vec3 dummy_vec3 = { 0.f, 0.0f, 0.0f };
    Vec4 dummy_vec4 = { 0.5f, 0.5f, 0.5f, 1.0f };
    
    for (size_t idx = 0; idx < count; ++idx)
    {
        vertices[idx].Position = MakeVec3(position_buffer + idx * 3);
        vertices[idx].Normals  = (normal_buffer) ? MakeVec3(normal_buffer + idx * 3) : dummy_vec3;
        vertices[idx].Color    = (color_buffer)  ? MakeVec4(color_buffer + idx * 4)  : dummy_vec4;
        vertices[idx].Tex0     = (uv0_buffer)    ? MakeVec2(uv0_buffer + idx * 2)    : dummy_vec2;
#if 0
        printf("Vertex %d\n", i);
        printf("\tPosition %lf %lf %lf\n", vertices[idx].Position.x, vertices[idx].Position.y, vertices[idx].Position.z);
        printf("\tNormals  %lf %lf %lf\n", vertices[idx].Normals.x, vertices[idx].Normals.y, vertices[idx].Normals.z);
        printf("\tColor    %lf %lf %lf %lf\n", vertices[idx].Color.x, vertices[idx].Color.y, vertices[idx].Color.z, vertices[i].Color.w);
        printf("\tTex0     %lf %lf\n", vertices[idx].Tex0.x, vertices[idx].Tex0.y);
#endif
    }
}

file_internal void ParseSkinnedVertex(SkinnedVertex *vertices,
                                      float  *position_buffer,
                                      float  *normal_buffer,
                                      float  *color_buffer,
                                      float  *uv0_buffer,
                                      float  *weights_buffer,
                                      float  *joints_buffer,
                                      size_t count)
{
    Vec2 dummy_vec2 = { 0.0f, 0.0f };
    Vec3 dummy_vec3 = { 0.f, 0.0f, 0.0f };
    Vec4 dummy_vec4 = { 0.5f, 0.5f, 0.5f, 1.0f };
    
    for (size_t idx = 0; idx < count; ++idx)
    {
        vertices[idx].Position = MakeVec3(position_buffer + idx * 3);
        vertices[idx].Normals  = (normal_buffer) ? MakeVec3(normal_buffer + idx * 3) : dummy_vec3;
        vertices[idx].Color    = (color_buffer)  ? MakeVec4(color_buffer + idx * 4)  : dummy_vec4;
        vertices[idx].Tex0     = (uv0_buffer)    ? MakeVec2(uv0_buffer + idx * 2)    : dummy_vec2;
        vertices[idx].Weights  = MakeVec4(weights_buffer + idx * 4);
        vertices[idx].Joints   = MakeVec4(joints_buffer  + idx * 4);
        
#if 0
        printf("Vertex %d\n", i);
        printf("\tPosition %lf %lf %lf\n", vertices[idx].Position.x, vertices[idx].Position.y, vertices[idx].Position.z);
        printf("\tNormals  %lf %lf %lf\n", vertices[idx].Normals.x, vertices[idx].Normals.y, vertices[idx].Normals.z);
        printf("\tColor    %lf %lf %lf %lf\n", vertices[idx].Color.x, vertices[idx].Color.y, vertices[idx].Color.z, vertices[i].Color.w);
        printf("\tTex0     %lf %lf\n", vertices[idx].Tex0.x, vertices[idx].Tex0.y);
#endif
    }
}

void MeshLoader::ParsePrimitive(cgltf_primitive *cg_primitive, Primitive *primitive)
{
    // this where the cool stuff happens
    
    // parse the material
    if (cg_primitive->material) {
        MaterialParameters mat_param = {};
        ParseMaterial(cg_primitive->material, &mat_param);
        
        primitive->MaterialId = CreateMaterial(mat_param);
    }
    
    if (cg_primitive->indices)
        primitive->IndexCount = cg_primitive->indices->count;
    else
        primitive->IndexCount = 0;
    
    // figure out the vertex count
    for (size_t k = 0; k < cg_primitive->attributes_count; ++k)
    { // find the position attribute
        cgltf_attribute attrib = cg_primitive->attributes[k];
        
        if (attrib.type == cgltf_attribute_type_position)
        {
            cgltf_accessor *accessor = attrib.data;
            
            primitive->VertexCount = attrib.data->count;
            
            // TODO(Dustin): If the Min/Max is not provided need to manually
            // track the min/max points while copying over the vertex data.
            if (accessor->has_min)
            {
                primitive->Min = MakeVec3(accessor->min);
            }
            
            if (accessor->has_max)
            {
                primitive->Max = MakeVec3(accessor->max);
            }
            
            break;
        }
    }
    
    size_t data_size = sizeof(u32) * primitive->IndexCount + sizeof(Vertex) * primitive->VertexCount;
    
    primitive->IndicesOffset = 0;
    primitive->VerticesOffeset = sizeof(u32) * primitive->IndexCount;
    
    primitive->DataBlock = mm::jalloc<char>(*GlobalModelAllocator, data_size);
    
    bool HasPosition = false;
    bool HasNormals  = false;
    bool HasColor    = false;
    bool HasUVs      = false;
    bool HasWeights  = false;
    bool HasJoints   = false;
    
    float *position_buffer = nullptr;
    float *normal_buffer   = nullptr;
    float *color_buffer    = nullptr;
    float *uv0_buffer      = nullptr;
    float *weights_buffer  = nullptr;
    float *joints_buffer   = nullptr;
    
    // Determine the attributes and get the ptr to their buffers
    for (size_t k = 0; k < cg_primitive->attributes_count; ++k)
    { // find the position attribute
        cgltf_attribute attrib = cg_primitive->attributes[k];
        
        cgltf_accessor *accessor = attrib.data;
        cgltf_buffer_view *buffer_view = accessor->buffer_view;
        
        // NOTE(Dustin): Might have to use top level buffer rather than buffer view
        char *buffer = (char*)buffer_view->buffer->data;
        
        switch (attrib.type)
        {
            case cgltf_attribute_type_position:
            {
                HasPosition = true;
                position_buffer = (float*)(buffer + accessor->offset + buffer_view->offset);
#if 0
                printf("Copied Positions:\n----------------------------------------\n");
                float *iter = position_buffer;
                for (int i = 0; i < primitive->VertexCount; ++i)
                {
                    printf("Vertex %d\n", i);
                    printf("\tPosition %lf %lf %lf\n", (*iter), (*iter), (*iter));
                    iter += 3;
                }
                printf("----------------------------------------\n");
#endif
            } break;
            
            case cgltf_attribute_type_normal:
            {
                HasNormals = true;
                normal_buffer = (float*)(buffer + accessor->offset + buffer_view->offset);
                
#if 0
                printf("Copied Normals:\n----------------------------------------\n");
                float *iter = normal_buffer;
                for (int i = 0; i < primitive->VertexCount; ++i)
                {
                    printf("Vertex %d\n", i);
                    printf("\tNormals  %lf %lf %lf\n", (*iter), (*iter), (*iter));
                    
                    iter += 3;
                }
                printf("----------------------------------------\n");
#endif
            } break;
            
            case cgltf_attribute_type_color:
            {
                HasColor = true;
                color_buffer = (float*)(buffer + accessor->offset + buffer_view->offset);
            } break;
            
            case cgltf_attribute_type_texcoord:
            {
                HasUVs = true;
                uv0_buffer = (float*)(buffer + accessor->offset + buffer_view->offset);
            } break;
            
            case cgltf_attribute_type_weights:
            {
                HasUVs = true;
                weights_buffer = (float*)(buffer + accessor->offset + buffer_view->offset);
            } break;
            
            case cgltf_attribute_type_joints:
            {
                HasUVs = true;
                joints_buffer = (float*)(buffer + accessor->offset + buffer_view->offset);
            } break;
            
            // TODO(Dustin): tangents,
            default: break;
        }
    }
    
    if (!HasPosition || !HasNormals)
    { /* TODO(Dustin): LOG */ }
    
    // Retrieve the index buffer data
    u32 *indices = nullptr;
    if (primitive->IndexCount > 0)
    {
        indices = (u32*)primitive->DataBlock + primitive->IndicesOffset;
        cgltf_accessor *indices_accessor = cg_primitive->indices;
        cgltf_buffer_view *indices_buffer_view = indices_accessor->buffer_view;
        
        // NOTE(Dustin): Might have to use top level buffer rather than buffer view
        char *unknown_indices_buffer = (char*)indices_buffer_view->buffer->data;
        
        // Copy the indices over
        // NOTE(Dustin): It is possible that the index type is not u16, so need
        // to verify its type
        switch (indices_accessor->component_type)
        {
            case cgltf_component_type_r_16u:
            {
                u16 *indices_buffer = (u16*)(unknown_indices_buffer +
                                             //indices_accessor->offset +
                                             indices_buffer_view->offset);
                for (int i = 0; i < primitive->IndexCount; ++i)
                {
                    indices[i] = (u32)indices_buffer[i];
                }
            } break;
            
            case cgltf_component_type_r_8u:
            {
                u8 *indices_buffer = (u8*)unknown_indices_buffer;
                
                for (int i = 0; i < primitive->IndexCount; ++i)
                {
                    indices[i] = (u32)indices_buffer[i];
                }
            } break;
            
            case cgltf_component_type_r_32u:
            {
                u32 *indices_buffer = (u32*)unknown_indices_buffer;
                
                for (int i = 0; i < primitive->IndexCount; ++i)
                {
                    indices[i] = (u32)indices_buffer[i];
                }
            } break;
            
            default:
            { /* TODO(Dustin): Might want to Log the invalid index size */ } break;
        }
        
#if 0
        // TEST CODE
        u16 *iter = (u16*)((char*)primitive->DataBlock + primitive->IndicesOffset);
        for (int i = 0; i < primitive->IndexCount; i += 3)
        {
            printf("%d %d %d\n", *(iter+0), *(iter+1), *(iter+3));
            
            iter += 3;
        }
        printf("----------------------------------------\n");
#endif
    }
    
    
    // Copy the vertex data over to its memory block
    size_t render_comp_stride = 0;
    if (joints_buffer && weights_buffer)
    {
        render_comp_stride = sizeof(SkinnedVertex);
        
        SkinnedVertex *vertices = (SkinnedVertex*)((char*)primitive->DataBlock + sizeof(u32) * primitive->IndexCount);
        
        ParseSkinnedVertex(vertices,
                           position_buffer,
                           normal_buffer,
                           color_buffer,
                           uv0_buffer,
                           weights_buffer,
                           joints_buffer,
                           primitive->VertexCount);
        
        
    }
    else
    {
        render_comp_stride = sizeof(Vertex);
        
        Vertex *vertices = (Vertex*)((char*)primitive->DataBlock + sizeof(u32) * primitive->IndexCount);
        
        ParseVertex(vertices,
                    position_buffer,
                    normal_buffer,
                    color_buffer,
                    uv0_buffer,
                    primitive->VertexCount);
    }
    
    
    primitive->RenderComp = CreateRenderComponent(primitive->VertexCount,
                                                  render_comp_stride,
                                                  ((char*)primitive->DataBlock + sizeof(u32) * primitive->IndexCount),
                                                  primitive->IndexCount,
                                                  sizeof(u32),
                                                  indices);
}

void MeshLoader::ParseMesh(cgltf_mesh *cg_mesh, Mesh *mesh)
{
    mesh->entity      = jengine::ecs::CreateEntity();
    mesh->HasSkeleton = false;
    
    mesh->PrimitivesCount = cg_mesh->primitives_count;
    mesh->Primitives = mm::jalloc<Primitive>(*GlobalModelAllocator, mesh->PrimitivesCount);
    
    for (int i = 0; i < cg_mesh->primitives_count; ++i)
    {
        ParsePrimitive(cg_mesh->primitives + i, mesh->Primitives + i);
    }
}

void MeshLoader::ParseNode(cgltf_node *cg_node, MeshNode *node)
{
    if (cg_node->name)
    {
        node->Name = pstring(cg_node->name);
    }
    
    if (cg_node->has_translation)
    {
        node->HasTranslation = true;
        node->Translation = MakeVec3(cg_node->translation);
    }
    else
    {
        node->HasTranslation = false;
        node->Translation = {0.0f,0.0f,0.0f};
    }
    
    if (cg_node->has_scale)
    {
        node->HasScale = true;
        node->Scale = MakeVec3(cg_node->scale);
    }
    else
    {
        node->HasScale = false;
        node->Scale = {1.0f,1.0f,1.0f};
    }
    
    if (cg_node->has_rotation)
    {
        node->HasRotation = true;
        node->Rotation = MakeVec4(cg_node->rotation);
    }
    else
    {
        node->HasRotation = false;
        node->Rotation = {0.0f,0.0f,0.0f,0.0f};
    }
    
    if (cg_node->has_matrix)
    {
        node->HasMatrix = true;
        node->Matrix = MakeMat4(cg_node->matrix);
    }
    else
    {
        node->HasMatrix = false;
        node->Matrix = Mat4(1.0f);
    }
    
    // Pointer to a mesh, if one exists
    if (cg_node->mesh)
    {
        cgltf_mesh *cg_mesh = cg_node->mesh;
        
        node->NodeMesh = mm::jalloc<Mesh>(*GlobalModelAllocator, 1);
        ParseMesh(cg_mesh, node->NodeMesh);
    }
    else
    {
        node->NodeMesh = nullptr;
    }
    
    // Recurse down the set to parse all child nodes
    node->ChildrenCount = cg_node->children_count;
    node->Children = mm::jalloc<MeshNode>(*GlobalModelAllocator, node->ChildrenCount);
    
    for (int i = 0; i < cg_node->children_count; ++i)
    {
        cgltf_node *child_cg_node = cg_node->children[i];
        
        NodeMappping.Insert(child_cg_node, node->Children + i);
        
        node->Children[i] = {};
        
        ParseNode(child_cg_node, node->Children + i);
    }
}

// A scene node is obtained from the scenes list
void MeshLoader::ParseScene(cgltf_scene *scene, Model *model)
{
    // Initialize model  memory
    model->nodes_count = (u32)scene->nodes_count;
    model->nodes = mm::jalloc<MeshNode>(*GlobalModelAllocator, model->nodes_count);
    
    // parse each disjoint set in this model
    for (int i = 0; i < scene->nodes_count; ++i)
    {
        *(model->nodes + i) = {};
        (model->nodes + i)->Parent = nullptr;
        
        NodeMappping.Insert(scene->nodes[i], model->nodes + i);
        
        // Parse the node and its descendents
        ParseNode(scene->nodes[i], model->nodes + i);
    }
}

file_internal Mesh* FindMesh(MeshNode *node, jstring &name)
{
    if (name == node->Name)
    {
        return node->NodeMesh;
    }
    
    Mesh *node_found = nullptr;
    for (int child = 0; child < node->ChildrenCount; ++child)
    {
        if (node_found = FindMesh(node->Children + child, name))
            break;
    }
    
    return node_found;
}

Mesh* FindMesh(jstring &name)
{
    for (u32 i = 0; i < GlobalCurrentModelCount; ++i)
    {
        Model model = ModelRegistry[i];
        
        for (u32 j = 0; j < model.nodes_count; ++j)
        {
            Mesh *found_mesh = nullptr;
            if (found_mesh = FindMesh(model.nodes+j, name))
            {
                return found_mesh;
            }
        }
    }
    
    return nullptr;
}

file_internal void AttachSkeletonToMesh(Mesh *mesh, SkeletonComponent *skeleton)
{
    mesh->HasSkeleton = true;
    mesh->Skeleton    = *skeleton;
}

file_internal void AttachAnimationToMesh(Mesh *mesh, Animation *animation, size_t animation_idx)
{
    mesh->Skeleton.Animations[animation_idx] = *animation;
}

void MeshLoader::LoadMesh(Model **models, size_t *model_count)
{
    cgltf_options options = {0};
    Data = nullptr;
    cgltf_result result = cgltf_parse_file(&options, Filename.GetCStr(), &Data);
    if (result != cgltf_result_success)
    {
        printf("Failed to load mesh %s!\n", Filename.GetCStr());
        return;
    }
    
    // load the buffers
    // NOTE(Dustin): Maybe in the future, stream the data directly to the gpu?
    result = cgltf_load_buffers(&options, Data, Filename.GetCStr());
    if (result != cgltf_result_success)
    {
        printf("Failed to load mesh buffers %s!\n", Filename.GetCStr());
        return;
    }
    
    NodeMappping = HashTable<cgltf_node*, MeshNode*>((u32)Data->nodes_count);
    
    *model_count = Data->scenes_count;
    
    if (GlobalCurrentModelCount + Data->scenes_count >= MAX_MODELS_IN_REGISTRY)
    {
        // TODO(Dustin): LOG out of model memory
        return;
    }
    
    for (int i = 0; i < Data->scenes_count; ++i)
    {
        cgltf_scene *cg_scene = &Data->scenes[i];
        ParseScene(cg_scene, &ModelRegistry[GlobalCurrentModelCount++]);
    }
    
    Animation *anims = talloc<Animation>(Data->animations_count);
    for (int i = 0; i < Data->animations_count; ++i)
    {
        ParseAnimation(Data->animations + i, anims + i);
    }
    
    // TODO(Dustin):
    SkeletonComponent skeleton = {};
    for (int skin = 0; skin < Data->skins_count; ++skin)
    {
        cgltf_skin cg_skin = Data->skins[skin];
        cgltf_node **joint_list = cg_skin.joints;
        cgltf_accessor *ibm_accessor = cg_skin.inverse_bind_matrices;
        
        Mat4 *ibm_buffer = (Mat4*)((char*)ibm_accessor->buffer_view->buffer +
                                   ibm_accessor->buffer_view->offset + ibm_accessor->offset);
        
        skeleton.IndexNodeMappings = mm::jalloc<MeshNode*>(*GlobalModelAllocator, cg_skin.joints_count);
        skeleton.IndexIBMMappings  = mm::jalloc<Mat4>(*GlobalModelAllocator,  cg_skin.joints_count);
        
        MeshNode *ptr = nullptr;
        if (!NodeMappping.Get(cg_skin.skeleton, &ptr))
        {
            printf("Failed to retrive the skeleton root pointer when parsing the skeleton!\n");
        }
        skeleton.SkeletonRoot = ptr;
        
        // TODO(Dustin): cgltf is reading in 0'd matrices for the IBM matrices. This does not seem right...
        for (int joint = 0; joint < cg_skin.joints_count; ++joint)
        {
            ptr = nullptr;
            if (!NodeMappping.Get(joint_list[joint], &ptr))
            {
                printf("Failed to retrive the joint pointer when parsing the skeleton!\n");
            }
            
            skeleton.IndexNodeMappings[joint] = ptr;
            skeleton.IndexIBMMappings[joint]  = ibm_buffer[joint];
        }
        
    }
    
    Mesh *fox_mesh = nullptr;
    if (!(fox_mesh = FindMesh(tstring("fox"))))
    {
        printf("Unable to find the mesh named fox!\n");
    }
    else
    {
        skeleton.Animations      = palloc<Animation>(Data->animations_count);
        skeleton.AnimationsCount = Data->animations_count;
        AttachSkeletonToMesh(fox_mesh, &skeleton);
        
        for (int anim_idx = 0; anim_idx < Data->animations_count; ++anim_idx)
        {
            AttachAnimationToMesh(fox_mesh, &anims[anim_idx], anim_idx);
        }
    }
    
    fox_mesh = FindMesh(tstring("fox"));
}

void LoadMesh(Model **models, size_t *model_count, jstring &filename) {
    
    MeshLoader ml = MeshLoader(filename);
    ml.LoadMesh(models, model_count);
}
