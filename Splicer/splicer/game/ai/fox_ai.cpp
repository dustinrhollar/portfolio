
using namespace jengine::ecs;

void FoxAiSystem::Update(u32 key_bitfield)
{
    ComponentIter<FoxAiComponent> fox_iter = GetComponentIter<FoxAiComponent>();
    
    FoxAiComponent *fox_ai = nullptr;
    while ((fox_ai = fox_iter.next()))
    {
        AnimationComponent anim_comp = {};
        anim_comp.CurrentTime = 0.0f;
        anim_comp.entity = fox_ai->MeshEntity;
        anim_comp.Skeleton = &fox_ai->Skeleton;
        anim_comp.CurrentKeyFrameIndex = 0;
        
        if (key_bitfield & KEY_PRESS_I)
        {
            // Find active animation
            for (int anim = 0; anim < fox_ai->Skeleton.AnimationsCount; ++anim)
            {
                if (fox_ai->Skeleton.Animations[anim].Name == fox_ai->BehaviorNames[0])
                {
                    anim_comp.ActiveAnimation = &fox_ai->Skeleton.Animations[anim];
                }
            }
            
            // TODO(Dustin): Retrieve a component from an entity to see if an animiation
            // is currently playing
            AddEntityToComponent<AnimationComponent>(fox_ai->MeshEntity, &anim_comp);
        }
        else if (key_bitfield & KEY_PRESS_O)
        {
            // Find active animation
            for (int anim = 0; anim < fox_ai->Skeleton.AnimationsCount; ++anim)
            {
                if (fox_ai->Skeleton.Animations[anim].Name == fox_ai->BehaviorNames[1])
                {
                    anim_comp.ActiveAnimation = &fox_ai->Skeleton.Animations[anim];
                }
            }
            
            anim_comp.CurrentKeyFrameIndex = 0;
            
            // TODO(Dustin): Retrieve a component from an entity to see if an animiation
            //is currently playing
            AddEntityToComponent<AnimationComponent>(fox_ai->MeshEntity, &anim_comp);
        }
        else if (key_bitfield & KEY_PRESS_P)
        {
            // Find active animation
            for (int anim = 0; anim < fox_ai->Skeleton.AnimationsCount; ++anim)
            {
                if (fox_ai->Skeleton.Animations[anim].Name == fox_ai->BehaviorNames[2])
                {
                    anim_comp.ActiveAnimation = &fox_ai->Skeleton.Animations[anim];
                }
            }
            
            anim_comp.CurrentKeyFrameIndex = 0;
            
            // TODO(Dustin): Retrieve a component from an entity to see if an animiation
            // is currently playing
            AddEntityToComponent<AnimationComponent>(fox_ai->MeshEntity, &anim_comp);
        }
    }
}