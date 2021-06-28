
using namespace jengine::mm;
using namespace jengine::ecs;

file_global u32 GlobalClientWidth;
file_global u32 GlobalClientHeight;

file_global PlayerCamera GlobalPlayerCamera;
file_global bool GlobalDevMode = true;

file_global bool GameInitialized = false;

file_internal void ProcessPlatformInput(u32 input_bitfield,
                                        r32  mouse_xpos,
                                        r32 mouse_ypos,
                                        bool dev_mode);
file_internal void CreateWorld();

platform_api Platform;

GAME_RESIZE(GameResize)
{
    if (!GameInitialized) {
        //glViewport(0, 0, width, height);
        GlobalClientWidth = width;
        GlobalClientHeight = height;
        
        // Resize the frontend renderer info
        // FrontendRendererResize(GlobalClientWidth, GlobalClientHeight);
        FrontEndRendererNeedsResized = true;
    }
}

GAME_SHUTDOWN(GameShutdown)
{
    ShutdownFrontEndRenderer();
    ShutdownECS();
    ShutdownMemoryManager();
}


GAME_INITIALIZE(GameInitialize)
{
    Platform = platform;
    
    GameInitialized = false;
    
    InitializeMemoryManager(_256MB * 2, _32MB);
    
#if 0
    printf("----------------------------------------\n");
    printf("Begin ALLOCATOR TESTING\n");
    
    struct AllocTester {
        void *ptr;
        bool active = false;
    };
    AllocTester Test[4096];
    
    srand(time(0));
    
    int amount = 4096;
    
    printf("Full allocation.\n");
    for (int i = 0; i < amount; ++i) {
        int idx = rand() % amount;
        {
            Test[i].ptr = palloc(idx);
            Test[i].active = true;
        }
    }
    
    printf("Full free.\n");
    for (int i = 0; i < amount; ++i) {
        {
            // printf("Idx free %i\n", i);
            pfree(Test[i].ptr);
            Test[i].active = false;
        }
    }
    
    printf("First round allocation.\n");
    for (int i = 0; i < amount; ++i) {
        int idx = rand() % amount;
        
        if (!Test[idx].active) {
            Test[idx].ptr = palloc(idx);
            Test[idx].active = true;
        }
    }
    
    printf("First round free.\n");
    for (int i = 0; i < amount/2; ++i) {
        int idx = rand() % amount;
        
        if (Test[idx].active) {
            pfree(Test[idx].ptr);
            Test[idx].active = false;
        }
    }
    
    printf("Second round allocation.\n");
    for (int i = 0; i < amount/4; ++i) {
        int idx = rand() % amount;
        
        if (!Test[idx].active) {
            Test[idx].ptr = palloc(idx);
            Test[idx].active = true;
        }
    }
    
    printf("Second round free.\n");
    for (int i = 0; i < amount/8; ++i) {
        int idx = rand() % amount;
        
        if (Test[idx].active) {
            pfree(Test[idx].ptr);
            Test[idx].active = false;
        }
    }
    
    printf("Third round allocation.\n");
    for (int i = 0; i < amount/16; ++i) {
        int idx = rand() % amount;
        
        if (!Test[idx].active) {
            Test[idx].ptr = palloc(idx);
            Test[idx].active = true;
        }
    }
    
    printf("Last round free.\n");
    int count = 0;
    for (int i = 0; i < amount; ++i) {
        if (Test[i].active) {
            pfree(Test[i].ptr);
            Test[i].active = false;
        }
    }
    
    
    printf("END ALLOCATOR TESTING\n");
    printf("----------------------------------------\n\n");
    
#endif
    
#if 0
    printf("----------------------------------------\n\n");
    printf("START ALLOCATOR TESTING\n");
    
    int sa1[100];
    int sa2[50];
    int sa3[150];
    
    int *pa = palloc<int>(100);
    
    for (int i = 0; i < 150; ++i) {
        if (i < 50) {
            sa2[i] = i;
        }
        
        if (i < 100){
            pa[i] = i;
            sa1[i];
        }
        
        sa3[i] = i;
    }
    
    int *ra1 = prealloc<int>(pa, 150);
    
    for (int i = 0; i < 100; ++i) {
        assert(sa3[i] == ra1[i]);
    }
    
    pfree(ra1);
    
    printf("END ALLOCATOR TESTING\n");
    printf("----------------------------------------\n\n");
#endif
    
    InitializeECS();
    
    if (!InitializeFrontEndRenderer())
    {
        GameShutdown();
        return false;
    }
    
    CreateWorld();
    
    VkExtent2D extent = vk::GetSwapChainExtent();
    GlobalClientWidth = extent.width;
    GlobalClientHeight = extent.height;
    
    GameInitialized = true;
    
    printf("END GAME INITIALIZATION\n");
    printf("----------------------------------------\n");
    printf("Size of jstring %zd\n", sizeof(jstring));
    
    return true;
}

file_internal void CreateWorld()
{
    // Register Fox AI System + AI Component
    FoxAiSystem ai_sys = {};
    RegisterSystem<FoxAiSystem>(&ai_sys);
    RegisterComponent<FoxAiComponent>();
    
    // Register Animation System
    AnimationSystem animation_sys = {};
    RegisterSystem<AnimationSystem>(&animation_sys);
    RegisterComponent<AnimationComponent>();
    
    // Create player camera
    Vec3 pos = {0.0f, 0.0f, -2.0};
    GlobalPlayerCamera = PlayerCamera(pos);
    
    // Test the mesh loader now...
    Model *models = nullptr;
    size_t models_count = 0;
    
    jstring filename = tstring("models/Fox/glTF/Fox.gltf");
    //jstring filename = tstring("models/splicer/glTF/KupierRigSculpt14blendtest1.gltf");
    //jstring filename = jstring("models/Lantern/glTF/Lantern.gltf");
    //jstring filename = jstring("models/BlenderBox/glTF/gloriousfish.gltf");
    LoadMesh(&models, &models_count, filename);
    
    Mesh *fox_mesh = FindMesh(tstring("fox"));
    if (!fox_mesh)
    {
        printf("Hey! Could not find that mesh!\n");
    }
    else
    {
        FoxAiComponent ai_comp = {};
        ai_comp.BehaviorNames[0] = "Walk";
        ai_comp.BehaviorNames[1] = "Run";
        ai_comp.BehaviorNames[2] = "Survey";
        ai_comp.Skeleton = fox_mesh->Skeleton;
        ai_comp.MeshEntity = fox_mesh->entity;
        
        AddEntityToComponent(ai_comp.MeshEntity, &ai_comp);
    }
}


file_internal void ProcessPlatformInput(u32 input_bitfield,
                                        r32 mouse_xpos,
                                        r32 mouse_ypos,
                                        r32 delta_time,
                                        bool dev_mode)
{
    if (input_bitfield & KEY_PRESS_W)
    {
        GlobalPlayerCamera.ProcessKeyboardInput(CAMERA_FORWARD, delta_time);
    }
    if (input_bitfield & KEY_PRESS_S)
    {
        GlobalPlayerCamera.ProcessKeyboardInput(CAMERA_BACKWARD, delta_time);
    }
    if (input_bitfield & KEY_PRESS_A)
    {
        GlobalPlayerCamera.ProcessKeyboardInput(CAMERA_LEFT, delta_time);
    }
    if (input_bitfield & KEY_PRESS_D)
    {
        GlobalPlayerCamera.ProcessKeyboardInput(CAMERA_RIGHT, delta_time);
    }
    if (input_bitfield & KEY_PRESS_1)
    {
        printf("1 was pressed.\n");
    }
    if (input_bitfield & KEY_PRESS_2)
    {
        printf("2 was pressed.\n");
    }
    if (input_bitfield & KEY_PRESS_3)
    {
        printf("3 was pressed.\n");
    }
    if (input_bitfield & KEY_PRESS_4)
    {
        printf("4 was pressed.\n");
    }
    //
    if (dev_mode != GlobalDevMode && GlobalDevMode)
    { // mode has changed from dev to game mode
        // neeed to reset mouse position
        GlobalPlayerCamera.MouseXPos = mouse_xpos;
        GlobalPlayerCamera.MouseYPos = mouse_ypos;
        
        GlobalDevMode = !GlobalDevMode; // swap the mode
    }
    else if (dev_mode != GlobalDevMode && !GlobalDevMode)
    {
        GlobalDevMode = !GlobalDevMode; // swap the mode
        
        GlobalPlayerCamera.MouseXPos = (r32)GlobalClientWidth/2.0f;
        GlobalPlayerCamera.MouseYPos = (r32)GlobalClientHeight/2.0f;
        
        // Force an update to the camera vectors
        GlobalPlayerCamera.ProcessMouseInput(GlobalPlayerCamera.MouseXPos, GlobalPlayerCamera.MouseYPos,
                                             (r32)GlobalClientWidth, (r32)GlobalClientHeight);
    }
    
    //if (!dev_mode)
    { // mouse moves camera in player mode
        GlobalPlayerCamera.ProcessMouseInput(mouse_xpos, mouse_ypos,
                                             (r32)GlobalClientWidth, (r32)GlobalClientHeight);
    }
    
    FoxAiSystem *fox_sys = GetSystem<FoxAiSystem>();
    fox_sys->Update(input_bitfield);
}


GAME_UPDATE(GameUpdate)
{
    // TODO(Dustin): Clear Temp Storage immediately
    
    ProcessPlatformInput(game_state.input_bitfield,
                         game_state.mouse_xpos,
                         game_state.mouse_ypos,
                         game_state.time,
                         game_state.DevMode);
    
    AnimationSystem *animation_sys = GetSystem<AnimationSystem>();
    animation_sys->Update();
    
    Mat4 proj = PerspectiveProjection(90.0f, (float)GlobalClientWidth/(float)GlobalClientHeight, 0.1f, 1000.0f);
    
    RenderSystem *rsys = GetSystem<RenderSystem>();
    rsys->Update(GlobalPlayerCamera, proj);
}
