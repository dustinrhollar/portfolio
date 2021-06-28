#ifndef SPLICER_GAME_ANIMATIONS_H
#define SPLICER_GAME_ANIMATIONS_H

struct MeshNode;

enum ChannelType
{
    CHANNEL_TYPE_INVALID,
    CHANNEL_TYPE_ROTATION,
    CHANNEL_TYPE_TRANSLATION,
    CHANNEL_TYPE_SCALING,
    CHANNEL_TYPE_WEIGHTS,
};


struct Channel
{
    ChannelType Type;
    union
    {
        Vec3       Translation;
        Vec3       Scaling;
        Quaternion Rotation;
    };
    
    MeshNode *Joint;
};

struct KeyFrame {
    r32  StartTime;
    DynamicArray<Channel> Channels;
};

struct KeyFrameNode
{
    KeyFrameNode *next;
    
    KeyFrame key_frame;
};

struct KeyFrameList
{
    int Size;
    
    KeyFrameNode *Root;
};

struct KeyFrameListIterator
{
    KeyFrameList *KeyFrameList;
    KeyFrameNode **CurrentKeyFrameNode;
};

void CreateKeyFrameList(KeyFrameList *key_frame_list);
void CreateKeyFrameIterator(KeyFrameList *key_frame_list, KeyFrameListIterator *iterator);
void AddToKeyFrameList(KeyFrameListIterator *key_frame_list,
                       r32 start_time, Channel channel);
void CollapseKeyFrameListToArray(KeyFrameList *key_frame_list,
                                 KeyFrame *key_frames);
void DestroyKeyFrameList(KeyFrameList *key_frame_list);

// TODO(Dustin): Handle memory for animations on shutdown
struct Animation {
    jstring   Name;
    KeyFrame  *KeyFrames;
    size_t    KeyFramesCount;
};

struct AnimationSystem : public jengine::ecs::ISystem
{
    // TODO(Dustin): DATA
    
    
    void Update();
};

struct SkeletonComponent
{
    // BufferParameters JointUniforms;
    // DescriptorSetLayout
    // DescriptorSet
    // u32 stride
    
    // NOTE(Dustin): Right now we are going to have a descriptor
    // set descriptor for each animation render component, but in
    // the fduture, test to see if having one descriptor set is possible/better.
    // On DescriptorWriteUpdate - rebind the buffer the descriptor points to.
    
    MeshNode *SkeletonRoot;
    
    // Array of # of joins, contains Node*
    // Array of # of joints, contain IBM
    MeshNode **IndexNodeMappings;
    // Index to Inverse Bind Transform data
    Mat4 *IndexIBMMappings;
    
    Animation *Animations;
    size_t    AnimationsCount;
    
};

struct AnimationComponent : jengine::ecs::IComponent
{
    jengine::ecs::Entity entity;
    
    // jstring name -> unique id
    size_t CurrentTime;
    
    SkeletonComponent *Skeleton;
    Animation         *ActiveAnimation;
    u32               CurrentKeyFrameIndex;
    //KeyFrame          CurrentKeyFrame;
    //KeyFrame          NextKeyFrame;
};

#endif //SPLICER_GAME_ANIMATIONS_H
