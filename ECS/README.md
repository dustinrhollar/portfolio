# ECS

## About

The project is an experimental architecture for a generic Entity Component System. A user can register custom components and systems at compile time and determine how each system updates its components. 

The goal of this project is to separate entity data into compartmentalized containers. Imagine a large entity struct, containing information for:
- Health
- Status
- Inventory
- World transforms
- Render data
- ...and so on

When I want to update the entity's position, I only need data for world transform, however each time I access an entity, I am pulling all of its data into the cache. This has a falloff effect on the next entity that needs to update its transform because it also needs to pull all of its data into the cache, which could result in an unnecessary cache miss. While this would be an unnecessary concern if I was just updating a few entities, it would become a large problem if I was updating a thousand entities at once. The goal of separating entity data into components is to place data that is often accessed together into a single container. A system can then iterate over all entities with a specific component, only pulling the required data into the cache.

## Usage

A simple use case is shown below. For a more complex example, see the `example` directory.
```
// components must inherit from IComponent
struct FooComponent : public IComponent
{
  int a;
  int b;
  int c;
};

// systems must inherit from ISystem
struct TestSystem : ISystem
{
  void UpdateFoo() 
  {
    ComponentIter<FooComponent> iter = GetComponentIter<FooComponent>();
    FooComponent *comp = nullptr;
    while ( (comp = iter.next()) ) printf("Foo Component has a: %d b: %d c: %d\n", comp->a, comp->b, comp->c);
  }
};

// In main...

// Register Foo as a component
GUID fid = RegisterComponent<FooComponent>();

// Register Test as a system
RegisterSystem<TestSystem>();

// Create an entity and attach the Foo component to it
Entity entity = CreateEntity();
AddEntityToComponent<FooComponent>(entity, {0, 0, 0});

// Get the test system and update all entities for that system
TestSystem *tsys = GetSystem<TestSystem>();
tsys->Update();

```
