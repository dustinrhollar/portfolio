//
// Created by Dustin Hollar on 10/30/18.
// 
  
#ifndef TERRAINGENERATORTEST_APPLICATION_H
#define TERRAINGENERATORTEST_APPLICATION_H
  
enum GuiState
{
	GUI_STATE_NONE,
	GUI_STATE_ORBIT,
	GUI_STATE_PAN,
	GUI_STATE_DOLLY,
	GUI_STATE_FOCUS
};

class Application {
    public:
    int windowWidth, windowHeight;
    GLFWwindow* window;
    Camera camera;
    
    Application(GLFWwindow* _window, bool _hasGUI, int width, int height);
    
    // GUI Handling
    virtual void guiInit();
    virtual void guiNewFrame();
	virtual void guiRender();
	virtual void guiWindow()       = 0;
	virtual void guiEventHandler() = 0;
    
    // GUI State
    GuiState guiState;
    bool isWindowVisible;
    bool hasGui;
    
    virtual void shutdown();
    virtual void setupWorld()    = 0;
    virtual void render()        = 0;
};

class TerrainApp : public Application {
    
    private:
    
#if !WIREFRAME
    DeferredRenderInfo deferredRenderer;
#endif
    
    Terrain* terrain;
    
    public:
    virtual void setupWorld() override;
    virtual void render()     override;
    virtual void shutdown()   override;
    
    public:
    TerrainApp(GLFWwindow* _window, bool _hasGUI, 
               int width, int height);
    ~TerrainApp();
    
    // GUI Handling
	virtual void guiWindow()       override;
	virtual void guiEventHandler() override;
    
};

#if 0
class VoxelApp : public Application
{ 
    private:
    VoxelWorld* voxelTerrain;
    
    public:
    void setupWorld( unsigned int, unsigned int, SimplexNoise::NoiseOctaveSimulation, unsigned int );
    
    virtual void render()        override;
    virtual void shutdown()   override;
    
    public:
    VoxelApp(GLFWwindow* _window, bool _hasGUI, int width, int height);
    ~VoxelApp();
    
    // GUI Handling
	virtual void guiWindow()       override;
	virtual void guiEventHandler() override;
};

class SimpleApp : public Application
{
    private:
    Light directionalLight = {}; // single directional light
    Light *lightList;            // list of other types of lights
    Mesh  *meshList;             // list of meshes
    
    //MeshList  meshList;
    //LightList lightList;    // mixture of point and spot lights
    
    public:
    virtual void render()     override;
    virtual void shutdown()   override;
    virtual void setupWorld() override;
    
    public:
    SimpleApp(GLFWwindow* _window, bool _hasGUI, int width, int height);
    ~SimpleApp();
    
    // GUI Handling
	virtual void guiWindow()       override;
	virtual void guiEventHandler() override; 
    
};
#endif

// LOD SYSTEM
// -----------------------------------------
// Struct for a directional light.

// Stores a portion of the grid 
struct Pizza { 
    uint VAO, instance_VBO, VBO, EBO; 
    LODMesh* mesh;
};

class LODApp : public Application {   
     
    private:
	// heightmap
    uint heightmapTexture;
	float* heightmap;
    
	// grid
    Pizza slices[2];
     
	// shader
    Shader* terrainShader;
      
	// main user camera
    Camera second_camera; 
    
	/* Create camera, heightmaps, and mesh */
    void initOpenGL();
    
    virtual void setupWorld() override;
    virtual void render()     override;
    virtual void shutdown()   override;
    
    
    // User input functions
    void processInput(GLFWwindow *window);
    
    public:
    LODApp( GLFWwindow* _window, bool _hasGUI, int width, int height);
    ~LODApp(); 
	// Public function that starts the Application 
    void runApplication( unsigned int, unsigned int, unsigned int );
      
    // GUI Handling 
	virtual void guiWindow()       {};
	virtual void guiEventHandler() {};
    
    
};

// Control the running application
// ------------------------------------------
extern Application *ActiveApp;

void SetActiveApplication(Application *app);
Application *GetActiveApplication();

#endif //TERRAINGENERATORTEST_APPLICATION_H
