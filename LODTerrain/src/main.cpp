
// Mouse info
// TODO(Dustin): Do better.
global float lastX;
global float lastY;
global bool firstMouse = true;

// timing
global float deltaTime = 0.0f;	// time between current frame and last frame
global float lastFrame = 0.0f;

internal void framebuffer_size_callback(GLFWwindow* window, int width, int height);
internal void mouse_callback(GLFWwindow* window, double xpos, double ypos);
internal void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
internal void processInput(GLFWwindow *window);

// Size of the application window
global int windowWidth  = 1920;
global int windowHeight = 1080;

internal GLFWwindow *initWindow()
{
    // For now, don't do anything
    lastX = windowWidth / 2.0f;
    lastY = windowHeight / 2.0f;
    
    
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
    
    // glfw window creation
    // --------------------
    GLFWwindow* window;
    window = glfwCreateWindow(windowWidth, windowHeight, "Terrain Generator", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        // TODO make a throws
        return NULL;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        // TODO Make a throws statement
        return NULL;
    }
    
    return window;
}

void render()
{
    Application *app = GetActiveApplication();
    
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    
    while (!glfwWindowShouldClose(app->window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // input
        // -----
        processInput(app->window);
        
        // render
        // ------
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        app->render();
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(app->window);
        glfwPollEvents();
    }
}

int main() {
    // Init GLFW
    GLFWwindow* window = initWindow();
    if (window == NULL)
    {
        exit(1);
    }
    
    InitializePipelineManager();
    
    // Resolution of the generated heightmap texture
    int textureWidth  = 512;
    int textureHeight = 512;
    
    // height scale
    // 256x256, 40
    u32 scale = 40;
    
    // Set up noise simulation
    SimplexNoise::NoiseOctaveSimulation noiseSim;
    noiseSim.width         = textureWidth;             // Width of the texture
    noiseSim.height        = textureHeight;            // Height of the texture
    noiseSim.numOctaves    = 8;                        // Control the number of octaves used
    noiseSim.persistence   = 0.50f;                    // Control the roughness of the output
    noiseSim.low           = 0.00f;                    // Low output value
    noiseSim.high          = 1.00f;                    // Height output value
    noiseSim.exp           = 2.00f;                    // Controls the intensity of black to white
    noiseSim.dim           = noiseSim.TWODIMENSION;    // Define the dimension of noise
    
    // Size of a terrain chunk
    u32 terrainWidth  = 256;
    u32 terrainHeight = 256;
    
    // Run the program
    LODApp a( window, false, windowWidth, windowHeight );
    //TerrainApp a( window, false, windowWidth, windowHeight );
    
    // Set the live application
    SetActiveApplication(&a);
    
    render();
    
    Application *app = GetActiveApplication();
    app->shutdown();
    
    FreePipelineManager();
    glfwTerminate();
    
    return EXIT_SUCCESS;
}


void processInput(GLFWwindow *window)
{
    Application *app = GetActiveApplication();
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        app->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        app->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        app->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        app->camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        app->camera.ProcessKeyboard(PAN_LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        app->camera.ProcessKeyboard(PAN_RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        app->camera.ProcessKeyboard(PAN_UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        app->camera.ProcessKeyboard(PAN_DOWN, deltaTime);
}


//// glfw: whenever the window size changed (by OS or user resize) this callback function executes
//// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// User input functions
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    
    lastX = xpos;
    lastY = ypos;
    
    Application *app = GetActiveApplication();
    app->camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Application *app = GetActiveApplication();
    app->camera.ProcessMouseScroll(yoffset);
}
