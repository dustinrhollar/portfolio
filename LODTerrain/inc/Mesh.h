typedef enum
{
    DRAW_MODE_TRIANGLE,
    DRAW_MODE_TRIANGLE_ELEMENT,
    DRAW_MODE_TRIANGLE_STRIP,
    DRAW_MODE_TRIANGLE_STRIP_ELEMENT,
    // Instanced version
    DRAW_MODE_INSTANCED_TRIANGLE,
    DRAW_MODE_INSTANCED_TRIANGLE_ELEMENT,
    DRAW_MODE_INSTANCED_TRIANGLE_STRIP,
    DRAW_MODE_INSTANCED_TRIANGLE_STRIP_ELEMENT,
} EDrawMode;

typedef struct
{
    int pipelineId;
    EDrawMode drawMode;
    
    // buffers
    u32 VAO; // required
    u32 VBO; // required
    
    bool hasEBO;      // default is false
    u32 EBO;
    
    // Number of elements in the EBO or VBO
    i32 size;
    
    glm::mat4 model;
    glm::mat4 proj;
} Mesh;

typedef struct
{
    Pipeline *pipeline;
    EDrawMode drawMode;
    
    // TODO(Dustin): Extract buffers here?
    
    size_t size;
    
    glm::mat4 *model; // List of model matrices
    glm::mat4 proj;
    glm::mat4 view;
} MeshInstanced;

#define DEFAULT_MESH_LIST_SIZE 10
typedef struct 
{
    int size;
    int cap;
    Mesh *meshes;
} MeshList;

// Takes the id of the attached pipeline. 
// If a number less than 0 is provided, a default pipeline
// is chosen.
void CreateMesh(Mesh *mesh, int pipelineId, bool hasEBO);
void FreeMesh(Mesh *mesh);

void CreateMeshList(MeshList *list, int cap);
void AddMesh(MeshList *list, Mesh *mesh);
void FreeMeshList(MeshList *list);

void RenderMesh(Mesh *mesh, Light *list, Camera* camera);