
namespace jengine { namespace ecs {
        
        file_global u16 EntityRegistry[MAX_ENTITIES];
        file_global size_t LastEntityIndex;
        file_global LinkedList<EID> FreeIndices;
        file_global u32 MIN_FREE_INDICES = 1024;
        
        // Each entity gets a linked list of attached components
        file_global LinkedList<EID> EntityComponents[MAX_ENTITIES];
        
        void IntializeEntityRegistry()
        {
            LastEntityIndex = 0;
        }
        
        void ShutdownEntityRegistry()
        {
            for (int i = 0; i < MAX_ENTITIES; ++i)
            {
                EntityRegistry[i] = 0; // reset all generations to 0
                
                EntityComponents[i].~LinkedList();
            }
            
            LastEntityIndex = 0;
            
            FreeIndices.~LinkedList();
        }
        
        file_internal Entity GenerateEntity(u64 index, u64 generation)
        {
            Entity entity;
            u64 idx = (index << ENTITY_INDEX_MASK);
            u64 gen = ((generation >> ENTITY_INDEX_BITS) & ENTITY_GENERATION_MASK);
            entity.id = ((generation << ENTITY_INDEX_BITS)) | (index);
            return entity;
        }
        
        // Determines if the Entity is "alive"
        bool IsValidEntity(Entity entity)
        {
            u16 gen = EntityRegistry[entity.index()];
            return  gen == entity.generation();
        }
        
        void DestroyEntity(Entity entity)
        {
            if (IsValidEntity(entity))
            {
                u64 idx = entity.index();
                
                LinkedList<EID> *components = &EntityComponents[idx];
                EID uid;
                while (components->Size() > 0)
                {
                    components->Pop(uid);
                    RemoveEntityFromComponent(uid, entity);
                }
                
                ++EntityRegistry[idx];
                FreeIndices.Push(idx);
            }
        }
        
        Entity CreateEntity()
        {
            u64 index;
            if (FreeIndices.Size() > MIN_FREE_INDICES)
            {
                FreeIndices.Pop(index);
            }
            else
            {
                u16 default = 0;
                EntityRegistry[LastEntityIndex++] = (default);
                index = LastEntityIndex-1;
                assert(index < (((unsigned __int64)1)<<ENTITY_INDEX_BITS));
            }
            
            return GenerateEntity(index, EntityRegistry[index]);
        }
        
        void AttachComonentToEntity(Entity entity, EID component_id)
        {
            if (IsValidEntity(entity))
            {
                LinkedList<EID> *components = &EntityComponents[entity.index()];
                
                // Check for duplicate components
                for (int i = 0; i < components->Size(); ++i)
                {
                    EID id = components->Get(i);
                    
                    if (component_id == id)
                    { // duplicate found, no need to to keep searching or add
                        return;
                    }
                }
                
                components->PushBack(component_id);
            }
        }
        
        void DetachComponentFromEntity(Entity entity, EID component_id)
        {
            if (IsValidEntity(entity))
            {
                LinkedList<EID> *components = &EntityComponents[entity.index()];
                components->Remove(component_id);
            }
        }
        
        LinkedList<EID> *GetAttachedComponents(Entity entity)
        {
            if (IsValidEntity(entity))
                return &EntityComponents[entity.index()];
            else
                return nullptr;
            
        }
        
    } // ecs
} // jengine
