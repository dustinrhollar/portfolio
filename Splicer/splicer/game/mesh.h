#ifndef SPLICER_GAME_MESH_H
#define SPLICER_GAME_MESH_H

struct Model;
struct RenderComponent;

file_global const u32 MAX_MODELS_IN_REGISTRY = 32;
extern Model ModelRegistry[MAX_MODELS_IN_REGISTRY];
extern u32   GlobalCurrentModelCount;

// Mesh might have multiple primitives
struct Primitive
{
    // TODO(Dustin): Allow for primitves to share materials.
    //MaterialParameters Material;
    u32 MaterialId;
    
    size_t IndexCount;
    size_t VertexCount;
    
    size_t IndicesOffset;
    size_t VerticesOffeset;
    
    Vec3 Min;
    Vec3 Max;
    
    RenderComponent RenderComp;
    
    void *DataBlock; // Pointer to where the primitive data is organized in interleaved format
};

struct Mesh
{
    jengine::ecs::Entity entity;
    
    bool HasSkeleton;
    SkeletonComponent Skeleton;
    
    Primitive *Primitives;
    size_t PrimitivesCount;
};

struct MeshNode
{
    jstring Name = jstring();
    
    MeshNode *Parent;
    
    MeshNode *Children;
    size_t ChildrenCount;
    
    bool HasTranslation;
    bool HasRotation;
    bool HasScale;
    bool HasMatrix;
    
    // NOTE(Dustin): This is an experiment. I think that
    // a node can have EITHER a Translation,Rotation,and Scale
    // OR a Model Matrix.
    
    // NOTE(Dustin): When matrix is provided, it must be decomposable
    // to TRS, and cannot skew or shear.
    
    // NOTE(Dustin): When a node is targeted for animation, only TRS
    // properties will be present; matrix will not be present.
    
    Vec3 Translation;
    Vec3 Scale;
    Vec4 Rotation; // this is a quaternion
    
    Mat4 Matrix;
    
    // Pointer to a mesh, if one exists
    Mesh *NodeMesh;
};

struct Model
{
    // a disjoint set of nodes
    MeshNode *nodes;
    u32 nodes_count;
};



void InitializeModelManager(size_t size);
void ShutdownModelManager();

/*

A gltf mesh has a node based struct.
Let a single "node" be 1 mesh (?)

How to handle nodes and their children in
order to build a relationship heirarchy?

*/

void LoadMesh(Model **models, size_t *model_count, jstring &filename);

// find a mesh based on its name
Mesh* FindMesh(jstring &name);


#endif //SPLICER_GAME_MESH_H
