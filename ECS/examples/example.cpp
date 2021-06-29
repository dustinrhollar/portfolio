#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

#include <jackal_types.h>

#include <ecs.h>
#include <entity.h>
#include <component.h>
#include <system.h>

#include <stdio.h>

global u64 GlobalPerfCountFrequency;

using namespace jengine;
using namespace jengine::mm;

u64 Win32GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return (Result.QuadPart);

}

float Win32GetSecondsElapsed(u64 start, u64 end)
{
    float Result = ((float)(end - start) /
                    (float)GlobalPerfCountFrequency)*1e9;
    return(Result);
}

using namespace jengine::ecs;
int main(void)
{

    // setup timing information
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceCounter(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    
    InitializeMemoryManager(_256MB, _32MB);
    
    InitializeECS();

    // Components must inherit from IComponent
    struct FooComponent : public IComponent
    {
        int a;
        int b;
        int c;
    };

    struct GooComponent : public IComponent
    {
        int a;
        int b;
        int c;
    };

    struct ZooComponent : public IComponent
    {
        int a;
        int b;
        int c;
    };

    // Systems must inherit from ISystem
    // Handles updating the Foo and Goo components
    struct DoubleSystem : ISystem
    {
        void UpdateFoo() 
        {
            ComponentIter<FooComponent> iter = GetComponentIter<FooComponent>();
            FooComponent *comp = nullptr;
            while ( (comp = iter.next()) )
            {
                printf("Foo Component has a: %d b: %d c: %d\n", comp->a, comp->b, comp->c);
            }
        }
        void UpdateGoo() 
        {
            ComponentIter<GooComponent> iter = GetComponentIter<GooComponent>();
            GooComponent *comp = nullptr;
            while ( (comp = iter.next()) )
            {
                printf("Goo Component has a: %d b: %d c: %d\n", comp->a, comp->b, comp->c);
            }
        }
        void Update() {printf("\n"); UpdateFoo(); printf("\n"); UpdateGoo();}
    };

    // Handles updating the Zoo component
    struct SingleSystem : ISystem
    {
        int modifier;
        void UpdateZoo() 
        {
            ComponentIter<ZooComponent> iter = GetComponentIter<ZooComponent>();
            ZooComponent *comp = nullptr;
            while ( (comp = iter.next()) )
            {
                printf("Zoo Component has a: %d b: %d c: %d\n", comp->a, comp->b, comp->c);

                // These components get modified
                comp->a *= modifier;
                comp->b *= modifier;
                comp->c *= modifier;
            }
        }
        void Update() {printf("\n"); UpdateZoo();}
    };
    
    // Register the Components + Systems
    GUID fid = RegisterComponent<FooComponent>();
    GUID gid = RegisterComponent<GooComponent>();
    GUID zid = RegisterComponent<ZooComponent>();

    RegisterSystem<DoubleSystem>();

    SingleSystem ss_data;
    ss_data.modifier = 10;
    RegisterSystem<SingleSystem>(&ss_data);

    // Create some entities
    Entity entities[MAX_ENTITIES];
    for (int i = 0; i < MAX_ENTITIES; ++i)
    {
        entities[i] = CreateEntity();
    }

    // Create the components for each entity
    FooComponent fcomponents[MAX_ENTITIES];
    for (int i = 0; i < MAX_ENTITIES; ++i)
    {
        fcomponents[i].a = i+100;
        fcomponents[i].b = i+200;
        fcomponents[i].c = i+300;
    }

    GooComponent gcomponents[MAX_ENTITIES];
    for (int i = 0; i < MAX_ENTITIES; ++i)
    {
        gcomponents[i].a = i*100;
        gcomponents[i].b = i*200;
        gcomponents[i].c = i*300;
    }

    ZooComponent zcomponents[MAX_ENTITIES];
    for (int i = 0; i < MAX_ENTITIES; ++i)
    {
        zcomponents[i].a = i-100;
        zcomponents[i].b = i-200;
        zcomponents[i].c = i-300;
    }

    // Give Entity 0 all three components
    AddEntityToComponent<FooComponent>(entities[0], &fcomponents[0]);
    AddEntityToComponent<GooComponent>(entities[0], &gcomponents[0]);
    AddEntityToComponent<ZooComponent>(entities[0], &zcomponents[0]);
    assert(3 == GetAttachedComponents(entities[0])->Size());

    // Duplicate Components should not get added
    AddEntityToComponent<ZooComponent>(entities[0], &fcomponents[0]);
    assert(3 == GetAttachedComponents(entities[0])->Size());

    // Give Entity 1 the Goo component
    AddEntityToComponent<GooComponent>(entities[1], &gcomponents[1]);
    assert(1 == GetAttachedComponents(entities[1])->Size());

    // Give Entity 2 the Zoo component
    AddEntityToComponent<ZooComponent>(entities[2], &zcomponents[2]);
    assert(1 == GetAttachedComponents(entities[2])->Size());

    // Remove the Goo component from Entity 0
    RemoveEntityFromComponent<GooComponent>(entities[0]);
    assert(2 == GetAttachedComponents(entities[0])->Size());

    // Get the systems and update them
    DoubleSystem *dsys = GetSystem<DoubleSystem>();
    dsys->Update();

    SingleSystem *ssys = GetSystem<SingleSystem>();
    ssys->Update();

    // Getting the Single System again should yield different values
    ssys = GetSystem<SingleSystem>();
    ssys->Update();

    // Killing Entitiy 0 should result in all systems being changed
    DestroyEntity(entities[0]);

    dsys = GetSystem<DoubleSystem>();
    dsys->Update();

    ssys = GetSystem<SingleSystem>();
    ssys->Update();

    ShutdownECS();
    ShutdownMemoryManager();
    
    return (0);
}