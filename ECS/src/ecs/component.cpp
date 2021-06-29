#include "component.h"

#include <mm.h>
#include <string.h>

namespace jengine { namespace ecs { 

GUID STATIC_COMPONENT_GUID = 0;

struct ComponentCache
{
    Entity cache[MAX_ENTITIES];
    size_t size; // current size of the cache. Must always be less that MAX_ENTITIES
};

// A list of all components and their corresponding
// caches. The capcity/size is identical to the Component
// Registry.
global ComponentCache *ComponentCacheList;

struct ComponentElement
{
    void *components         = nullptr;
    size_t size_of_component = 0;
};

global ComponentElement *ComponentRegistry;
global size_t ComponentCount;
global size_t ComponentCapacity;

void InitializeComponentRegistry()
{
    ComponentRegistry = (ComponentElement*)mm::jalloc(10 * sizeof(ComponentElement));
    ComponentCapacity = 10;
    ComponentCount = 0;

    // clear the memory
    for (int i = 0; i < ComponentCapacity; ++i) 
    {
        ComponentRegistry[i].components = nullptr;
        ComponentRegistry[i].size_of_component = 0;
    }

    // Initialize the Component Cache
    ComponentCacheList = (ComponentCache*)mm::jalloc(ComponentCapacity * sizeof(ComponentCache));
}

void ShutdownComponentRegistry()
{
    for (int i = 0; i < ComponentCount; ++i)
    {
        mm::jfree(ComponentRegistry[i].components);
    }

    mm::jfree(ComponentRegistry);
    mm::jfree(ComponentCacheList);
    ComponentCapacity = 0;
    ComponentCount = 0;
}

void AddComponentToRegistry(GUID component_id, size_t size_per_component)
{
    // In case a user accidentally registers the same component twice
    if (ComponentRegistry[component_id].components != nullptr) return;
    // Each new component should be incremental in Id. So it should be added at "size"
    assert(component_id == ComponentCount);

    if (ComponentCount + 1 == ComponentCapacity)
    { // Resize the registry
        size_t new_cap = ComponentCapacity * 2; // amoritize add

        ComponentElement *ptr = (ComponentElement*)mm::jalloc(new_cap * sizeof(ComponentElement));
        ComponentCache *cache_ptr = (ComponentCache*)mm::jalloc(new_cap * sizeof(ComponentCache));
        for (int i = 0; i < ComponentCount; ++i)
        { // copy over the pointer to the memory blocks
            memcpy(&ptr[i], &ComponentRegistry[i], sizeof(ComponentElement));
            memcpy(&cache_ptr[i], &ComponentCacheList[i], sizeof(ComponentCache));
        }

        mm::jfree(ComponentRegistry);
        mm::jfree(ComponentCacheList);
        ComponentRegistry = ptr;
        ComponentCacheList = cache_ptr;
        ComponentCapacity = new_cap;
    }

    ComponentRegistry[ComponentCount].components = mm::jalloc(size_per_component * MAX_ENTITIES);
    ComponentRegistry[ComponentCount].size_of_component = size_per_component;    

    // Activate the component cache for this Component
    ComponentCacheList[ComponentCount].size = 0;

    ComponentCount++;

    // Go through each component and mark them as inactive
    for (int i = 0; i < MAX_ENTITIES; ++i)
    {
        ((IComponent*)((char*)ComponentRegistry[component_id].components + (size_per_component * i)))->IsActive = false;
    }
}

void* GetComponentFromRegistry(GUID component_id)
{
    return ComponentRegistry[component_id].components;
}

void AddEntityToComponent(GUID component_id, Entity entity, void *data, size_t size_of_component)
{
    u64 idx = entity.index();

    void *ptr = (void*)(((char*)ComponentRegistry[component_id].components + (size_of_component * idx)));

    if (((IComponent*)ptr)->IsActive)
    {
        // TODO: LOG that there is already an entity at this location...!
    }
    else 
    {
        // copy the component data to the entities assigned index in the component list
        
        memcpy(ptr, data, size_of_component);

        // All components should inherit from IComponent
        IComponent *icomp = (IComponent*)ptr;
        (icomp)->IsActive = true;

        ComponentCacheList[component_id].cache[ComponentCacheList[component_id].size++] = entity;   
    }
}

void RemoveEntityFromComponent(GUID component_id, Entity entity)
{
    u64 idx = entity.index();
    size_t size_of_component = ComponentRegistry[component_id].size_of_component;

    ((IComponent*)((char*)ComponentRegistry[component_id].components + (size_of_component * idx)))->IsActive = false;
}

void FlushComponentCache(GUID component_id)
{
    ComponentCache *cache_iter = &ComponentCacheList[component_id];
    ComponentElement *component = &ComponentRegistry[component_id];
    for (int i = 0; i < cache_iter->size; ++i)
    {
        Entity e = *(cache_iter->cache + i);

        // Get the entity fro mthe components
        if (!((IComponent*)((char*)component->components + (component->size_of_component * e.index())))->IsActive)
        { // the entity is not active, swap it with the last element in the cache
            *(cache_iter->cache + i) = *(cache_iter->cache + cache_iter->size - 1);
            cache_iter->size--;

            --i; // need to check the element that was swapped to make sure it is active too.
        }
    }
}

void *NextInCache(GUID component_id, size_t next_idx)
{
    ComponentCache *cache_iter = &ComponentCacheList[component_id];
    ComponentElement *component = &ComponentRegistry[component_id];
    
    while (next_idx < cache_iter->size)
    {
        Entity entity = cache_iter->cache[next_idx];

        void *component_data = (char*)component->components + (component->size_of_component * entity.index());

        if (!(((IComponent*)component_data)->IsActive))
        {
            *(cache_iter->cache + next_idx) = *(cache_iter->cache + cache_iter->size - 1);
            cache_iter->size--;
        }
        else
        {
            return component_data;
        }
    }

    return nullptr;
}

void *NextInCacheNoSwap(GUID component_id, size_t next_idx)
{
    ComponentCache *cache_iter = &ComponentCacheList[component_id];
    ComponentElement *component = &ComponentRegistry[component_id];

    if (next_idx >= cache_iter->size)
        return nullptr;
    
    Entity entity = cache_iter->cache[next_idx];
    return (char*)component->components + (component->size_of_component * entity.index());
}

} // ecs
} // jengine