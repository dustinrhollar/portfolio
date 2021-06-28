
// For now, let's say only 32 Materials can be loaded at once
file_global const u32 MAX_MATERIALS = 32;

// allows for 2^24 unique Materials. I seriously doubt there will ever be
// that many loaded :p
static const u32 MATERIAL_INDEX_BITS = 24;
static const u32 MATERIAL_INDEX_MASK = (((unsigned __int64)1)<<MATERIAL_INDEX_BITS)-1;

static const u32 MATERIAL_GENERATION_BITS = 8;
static const u32 MATERIAL_GENERATION_MASK = (((unsigned __int64)1)<<MATERIAL_GENERATION_BITS)-1;

struct MaterialEntity {
    u32 Id;
};

// An entity with an id of 0 is an invalid entity
#define INVALID_MATERIAL_ENTITY 0

// Used to determine uniqueness
file_global u8 MaterialGenerations[MAX_MATERIALS];

// Used to retrieve renderinfo for that material
file_global MaterialComponent MaterialRegistry[MAX_MATERIALS];

// used to retrive system information for a material (name of the material)
file_global jstring MaterialSystemInfo[MAX_MATERIALS];
file_global HashTable<jstring, MaterialEntity> MaterialSystemTable;

// Next index to insert
file_global u32 LastMaterialEntityIndex = 0;

// List for indices ready for re-use, treated as a stack for quick push/pop operations
file_global LinkedList<u32> MaterialFreeIndices;
// minimum available inidces before indices can be re-used
file_global size_t MinAllowedMaterialFreeIndices  = 8;

// Useful forward declarations
file_internal MaterialEntity GenerateMaterialEntity(u32 index, u32 generation);

inline u32 MaterialEntityToIndex(MaterialEntity entity) {
    return (entity.Id & MATERIAL_INDEX_MASK);
}

inline u32 MaterialEntityToGeneration(MaterialEntity entity) {
    return (entity.Id >> MATERIAL_INDEX_BITS) & MATERIAL_GENERATION_MASK;
}

file_internal MaterialEntity GenerateMaterialEntity(u32 index, u32 generation) {
    MaterialEntity entity;
    u32 idx   = (index << MATERIAL_INDEX_MASK);
    u32 gen   = ((generation >> MATERIAL_INDEX_BITS) & MATERIAL_GENERATION_MASK);
    entity.Id = ((generation << MATERIAL_INDEX_BITS)) | (index);
    return entity;
}

void InitializeMaterialManager() {
    
    MaterialSystemTable = HashTable<jstring, MaterialEntity>(MAX_MATERIALS);
    
    for (int i = 0; i < MAX_MATERIALS; ++i) {
        MaterialRegistry[i] = {};
    }
}

void ShutdownMaterialManager() {
    MaterialSystemTable.Reset();
    
    for (int i = 0; i < MAX_MATERIALS; ++i) {
        MaterialSystemInfo[i].Clear();
        MaterialRegistry[i].Material.Name.Clear();
    }
}


u32 CreateMaterial(MaterialParameters material_param) {
    MaterialEntity entity;
    if (MaterialSystemTable.Get(material_param.Name, &entity))
    {
        printf("Duplicate material found: %s\n", material_param.Name.GetCStr());
        return entity.Id;
    }
    
    // Generate a MaterialEntity
    u32 index = INVALID_MATERIAL_ENTITY;
    if (MaterialFreeIndices.Size() >= MinAllowedMaterialFreeIndices) {
        MaterialFreeIndices.Pop(index);
    }
    else {
        u8 default_gen = 0;
        index = LastMaterialEntityIndex;
        MaterialGenerations[index] = (default_gen);
        assert(index < (((unsigned __int32)1)<<MATERIAL_INDEX_BITS));
        
        LastMaterialEntityIndex++;
    }
    
    entity = GenerateMaterialEntity(index, MaterialGenerations[index]);
    
    
    MaterialComponent mat_comp = {};
    
    // TODO(Dustin): Create Per Material Descriptor Sets
    //VkDescriptorSetLayout *DescriptorSetLayouts;
    //u32                   DescriptorSetLayoutsCount;
    
    //VkDescriptorSet *DescriptorSets;
    //u32             DescriptorSetsCount;
    
    // Static buffers (i.e. Textures?)
    mat_comp.Material = material_param;
    
    // Insert the new entity into its various structures
    MaterialRegistry[index]   = mat_comp;
    MaterialSystemInfo[index] = material_param.Name;
    MaterialSystemTable.Insert(material_param.Name, entity);
    
    return entity.Id;
}


// True if valid (generations match), false otherwise
bool IsValidMaterial(u32 id) {
    MaterialEntity entity = {id};
    u32 idx = MaterialEntityToIndex(entity);
    u32 gen = MaterialEntityToGeneration(entity);
    
    if (idx >= MAX_MATERIALS)
        return false;
    return (MaterialGenerations[idx] == gen);
}

jstring GetMaterialName(u32 id) {
    MaterialEntity entity = {id};
    
    // if the material is not valid, return an empty string
    return (IsValidMaterial(entity.Id)) ? MaterialSystemInfo[MaterialEntityToIndex(entity)] : jstring();
}

MaterialComponent GetMaterialComponent(u32 id) {
    MaterialEntity entity = {id};
    
    // NOTE(Dustin): Silent Failure if the material is no longer valid
    MaterialComponent comp = {};
    if (IsValidMaterial(entity.Id))
        comp = MaterialRegistry[MaterialEntityToIndex(entity)];
    return comp;
}
