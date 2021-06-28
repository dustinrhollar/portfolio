#ifndef SPLICER_GAME_AI_FOX_AI_H
#define SPLICER_GAME_AI_FOX_AI_H

struct FoxAiSystem : public jengine::ecs::ISystem
{
    void Update(u32 key_bitfield);
};

struct FoxAiComponent : jengine::ecs::IComponent
{
    /*

3 behaviors for this AI:
--- 1. Walk
--- 2. Run
--- 3. Survey

Default: Idle

*/
    jstring              BehaviorNames[3];
    SkeletonComponent    Skeleton;
    jengine::ecs::Entity MeshEntity;
};

#endif //SPLICER_GAME_AI_FOX_AI_H
