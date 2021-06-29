//
// Application layer for the Terrain Render. Handles the creation of OpenGL state and the Render Loop.
// @author Dustin Hollar
//

//#define WIREFRAME
//#define CLIP
// Defines whether or not tessellation shaders are active
#define TESSELLATION
//#define MOVEMENT
//#define SECONDARY_CAMERA

static const uint patch_size = 16;
static const uint rows = 50;
static const float xy_Scale  = 20;
static const float z_Scale   = 500.0f;
static const float MinDistance = 1.0f;
static const float MaxDistance = 600;


// Heightmap data
static const int textureWidth  = 2048;
static const int textureHeight = 2048;

LODApp::LODApp( GLFWwindow* window, bool hasGUI, int width, int height ) :
Application( window, hasGUI, width, height ) 
{
	setupWorld();
    initOpenGL();
}

LODApp::~LODApp() = default;

// run the application
void LODApp::runApplication( unsigned int terrainWidth, unsigned int terrainHeight,
                            unsigned int scale ) {
	
    setupWorld();
    initOpenGL();
	//run();
}

// Set up camera and terrain
void LODApp::setupWorld() {
    
	//----------------------------------------------------------------------------------------------//
	// NOISE SIMULATION
	//----------------------------------------------------------------------------------------------//
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
    
	heightmap = simulateNoise(noiseSim);
    
	//----------------------------------------------------------------------------------------------//
	// CAMERA
	//----------------------------------------------------------------------------------------------//
	// Create camera based on heightmap
	uint x = textureWidth / 2;
	uint z = textureHeight / 2;
	float y = heightmap[(textureWidth * x) + z];
    
	y *= z_Scale;
	camera = Camera(glm::vec3(x, 3 * y, z));
	second_camera = Camera(glm::vec3(x, 0, z));
    //camera = new Camera;
	//----------------------------------------------------------------------------------------------//
	// MESH GENERATION
	//----------------------------------------------------------------------------------------------//
    // render two slices
    float lowerAngleSlice1 = 0.0f;
    float upperAngleSlice1 = 45.5f;
    
    float lowerAngleSlice2 = 22.5f;
    float upperAngleSlice2 = 45.5f;

    // Lower slice
    std::cout << "Generating slices..." << std::endl;
	slices[0].mesh = generateSlice( rows, patch_size, DEGREES, lowerAngleSlice1, upperAngleSlice1 );
    // slices[0].mesh = generateFullMesh( grid_width, grid_width, patch_size );
    
    // Upper slice
    slices[1].mesh = generateSlice( rows, patch_size, DEGREES, lowerAngleSlice2, upperAngleSlice2 );
}

// contains 4 2x2 matrices for transforming the mesh
static float transforms[] = {
    1,  0,  0,  1,  // 0 .. 45
    0,  1,  1,  0,  // 45 .. 90 
    0, -1,  1,  0,  // 90 .. 135
    1,  0,  0, -1,  // 135 .. 180
	-1,  0,  0, -1,  // 180 .. 225
    0, -1, -1,  0,  // 225 .. 270
    0,  1, -1,  0,  // 270 .. 315
	-1,  0,  0,  1   // 315 .. 360
};

// set up shaders
void LODApp::initOpenGL() {
    //    std::cout << "setting up buffers" << std::endl;
    
    // loader the terrain shader
    std::cout << "Loading terrain shaders..." << std::endl;
    
#ifdef  TESSELLATION
    terrainShader = new Shader(
        "shaders/lodVertexShader.vs",
        "shaders/lodFragmentShader.fs",
        "shaders/tessControl.tc",
        "shaders/tessellationEvaluation.te");
#else
    terrainShader = new Shader(
        "shaders/lodVertexShader.vs",
        "shaders/lodFragmentShader.fs");
#endif
    
    // ------------------------------------------------------------------------------------------//
    // PIZZA SLICE 1
    // ------------------------------------------------------------------------------------------//
    // Set up buffers
    glGenVertexArrays(1, &slices[0].VAO);
    glGenBuffers(1, &slices[0].VBO);
    //glGenBuffers(1, &slices[0].EBO);
    
    glBindVertexArray(slices[0].VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, slices[0].VBO);
	// The * 4 is for instancing
    glBufferData(GL_ARRAY_BUFFER, slices[0].mesh->size_vert, slices[0].mesh->verts, GL_STATIC_DRAW);
	/*
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, slices[0].EBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, slices[0].mesh->size_indices,
                  slices[0].mesh->indices, GL_STATIC_DRAW );
    */
    // position attribute
    glVertexAttribPointer( 0,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          2 * sizeof( float ),
                          (void *)0 );
    
	// Every instance update the buffer for the vertex attribute at 0
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    
	
    
	glGenBuffers(1, &slices[0].instance_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, slices[0].instance_VBO);
	// 2x2 = 4 floats, 7 instances = 7 * 4 * sizeof(float)
	glBufferData(GL_ARRAY_BUFFER, 8 * 4 * sizeof(float), &transforms[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribDivisor(1, 1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    
    // ------------------------------------------------------------------------------------------//
    // PIZZA SLICE 2
    // ------------------------------------------------------------------------------------------//
	// Set up buffers
	glGenVertexArrays(1, &slices[1].VAO);
	glGenBuffers(1, &slices[1].VBO);
	//glGenBuffers(1, &slices[0].EBO);
    
	glBindVertexArray(slices[1].VAO);
    
	glBindBuffer(GL_ARRAY_BUFFER, slices[1].VBO);
	// The * 4 is for instancing
	glBufferData(GL_ARRAY_BUFFER, slices[1].mesh->size_vert, slices[1].mesh->verts, GL_STATIC_DRAW);
	/*
 glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, slices[0].EBO );
 glBufferData( GL_ELEMENT_ARRAY_BUFFER, slices[0].mesh->size_indices,
      slices[0].mesh->indices, GL_STATIC_DRAW );
 */
	// position attribute
	glVertexAttribPointer(0,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          2 * sizeof(float),
                          (void *)0);
    
	// Every instance update the buffer for the vertex attribute at 0
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    
	glGenBuffers(1, &slices[1].instance_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, slices[1].instance_VBO);
	// 2x2 = 4 floats, 4 instances = 4 * 4 * sizeof(float)
	glBufferData(GL_ARRAY_BUFFER, 7 * 4 * sizeof(float), &transforms[1], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribDivisor(1, 1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
    
	// set up heightmap texture
	int textureWidth = 2048;
	int textureHeight = 2048;
	glGenTextures(1, &heightmapTexture);
	glBindTexture(GL_TEXTURE_2D, heightmapTexture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
	if (heightmap) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, textureWidth, textureHeight, 0, GL_RED, GL_FLOAT, heightmap);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
    
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

// Function that returns the transform for a slice of the grid mesh
glm::vec4 getSlice(int i) {
    
	switch (i) {
        case 0:
		return glm::vec4(1, 0, 0, 1);
		
        case 1:
		return glm::vec4(0, 1, 1, 0);
		
        case 2:
		return glm::vec4(0, -1, 1, 0);
		
        case 3:
		return glm::vec4(1, 0, 0, -1);
		
        case 4:
		return glm::vec4(-1, 0, 0, -1);
		
        case 5:
		return glm::vec4(0, -1, -1, 0);
		
        case 6:
		return glm::vec4(0, 1, -1, 0);
		
        case 7:
		return glm::vec4(-1, 0, 0, 1);
		
		default: return glm::vec4();
	}
}

// render loop
void LODApp::render() {    
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    
#ifdef WIREFRAME
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
    
	// Camera used for simulation
	//Camera c_pos = Camera(glm::vec3(camera->Position[0], camera->Position[1], camera->Position[2]));
	//c_pos.Yaw = -45.0;
    
	// Boolean array for culling
#ifdef CLIP
	uint len = 8;
	int canRender[0]; memset(canRender, 0, sizeof(int)*8);
#endif
    
	// used for patch interpolation in the tessellation evaluation shader
	glm::mat4 bezier = glm::mat4(-1, 3, -3, 1,
                                 3, -6, 3, 0,
                                 -3, 3, 0, 0,
                                 1, 0, 0, 0);
    
    // render loop
    // -----------
    //while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        //float currentFrame = glfwGetTime();
        //deltaTime = currentFrame - lastFrame;
        //lastFrame = currentFrame;
        
        // input
        // -----
        //processInput(window);
        
		// Input for the secondary camera
#ifdef MOVEMENT
		second_camera.ProcessKeyboard(PAN_LEFT, 0.00167f);
		second_camera.ProcessKeyboard(FORWARD,  0.00167f);
#endif
        
        // render
        // ------
        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //slices[0].mesh
        // 1. geometry pass: render scene's geometry/color data into gbuffer
        // -----------------------------------------------------------------
        terrainShader->use(); 
        
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / (float)windowHeight, 1.f, 10000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        terrainShader->setMat4( "projection", projection );
        terrainShader->setMat4( "view", view );
        
        // calculate the model matrix for each object and pass it to shader before drawing
        glm::mat4 model = glm::mat4(1.0);
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        
		// Shader uniforms
#ifdef SECONDARY_CAMERA
		terrainShader->setVec3("camera", second_camera.Position);
		model = glm::translate(model, second_camera.Position);
#else 
		terrainShader->setVec3("camera", camera.Position);
		model = glm::translate(model, camera.Position);
#endif
		terrainShader->setMat4("model", model);

		terrainShader->setFloat("vertexSpacing", xy_Scale);
		terrainShader->setInt("scale", z_Scale);
        terrainShader->setVec3( "terrainOrigin", glm::vec3( 0, 0, 0) );
		terrainShader->setInt("width", 2048);
		terrainShader->setFloat("MaxDistance", MaxDistance);
		terrainShader->setFloat("MinDistance", MinDistance);
        glUniformMatrix4fv( glGetUniformLocation(terrainShader->ID, "B"), 1, GL_FALSE, &bezier[0][0]);
        glUniformMatrix4fv( glGetUniformLocation(terrainShader->ID, "BT"), 1, GL_TRUE, &bezier[0][0]);
        
		// Heightmap Texture
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(terrainShader->ID, "heightmap"), 0);
		glBindTexture(GL_TEXTURE_2D, heightmapTexture);
        
		//----------------------------------------------------------//
		// DETERMINE CLIPPING RANGES
		//----------------------------------------------------------//
#ifdef CLIP
		int angle = ( ( ( (int)camera.Yaw % 360 )));
        
		float idxf = angle / (float)45.0f;
		int idx;
		if ( idxf == 0 || abs(static_cast<int>(idxf) / idxf) < 0.5f || abs(static_cast<int>(idxf)) / idxf <= .00000000000000000000001f)
			idx = static_cast<int>(idxf);
		else {
			if( idxf > 0 )
				idx = static_cast<int>(idxf) - 1;
			else
				idx = static_cast<int>(idxf) + 1;
		}
        
		//std::cout << idx << std::endl;
		--idx;
		canRender[abs((-idx + 7) % 8)] = 1;
		canRender[ abs((-idx + 8) % 8) ] = 1;
		canRender[abs((-idx + 9) % 8)] = 1;
        
		if(static_cast<int>(idxf) / idxf <= -0.4f)
			canRender[abs((-idx + 6) % 8)] = 1;
		if (static_cast<int>(idxf) / idxf <= 0.8f)
			canRender[abs((-idx + 10) % 8)] = 1;
#endif
        
		//----------------------------------------------------------//
		// RENDER MESH
		//----------------------------------------------------------//
        for( int i = 0; i < 8; i ++ ) {
            
#ifdef CLIP
			if (canRender[i] == 0) {
				continue;
			}
#endif
            
            // Draw first slice
            glBindVertexArray( slices[0].VAO );
            
			glm::vec4 slice = getSlice(i);
            
            assert(slices[0].mesh != nullptr);
            
			terrainShader->setVec4( "g_transform", slice);
            
#ifdef TESSELLATION
            glPatchParameteri(GL_PATCH_VERTICES,  patch_size);
			//glDrawArraysInstanced(GL_PATCHES, 0, slices[0].mesh->num_vert, 8);
			glDrawArrays(GL_PATCHES, 0, slices[0].mesh->num_vert);
#else
            glDrawArrays(GL_QUADS, slices[0].mesh->num_vert, GL_UNSIGNED_INT );
#endif
            
            glBindVertexArray(0);
            
            // Draw second pizza slice
            //glBindVertexArray(slices[1].VAO);
            
			/*
            
#ifdef TESSELLATION
   glPatchParameteri(GL_PATCH_VERTICES, patch_size);
   //glDrawElements( GL_PATCHES, slices[0].mesh->num_indices,  GL_UNSIGNED_INT, nullptr );
   glDrawArraysInstanced(GL_PATCHES, 0, slices[1].mesh->num_vert, 7);
   // glDrawArrays(GL_QUADS, slices[1].mesh->num_vert, GL_UNSIGNED_INT);
#else
            //glDrawArrays(GL_QUADS, slices[1].mesh->num_vert, GL_UNSIGNED_INT );
#endif

            glBindVertexArray(0);
   */
            
			
        }
        
		// reset culling
#ifdef CLIP
		for (int i = 0; i < 8; ++i) {
			canRender[i] = 0;
		}
#endif
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        //glfwSwapBuffers(window);
        //glfwPollEvents();
    }
    
#ifdef CLIP
	//free(canRender);
#endif
    
    //shutdown();
}

// Free memory when the application exits
void LODApp::shutdown() {
    
    // shutdown buffers...TODO
    glDeleteVertexArrays( 1, &slices[0].VAO );
    glDeleteVertexArrays( 1, &slices[1].VAO );
    
    glDeleteBuffers( 1, &slices[0].VBO );
    glDeleteBuffers( 1, &slices[1].VBO );
	glDeleteBuffers(1, &slices[0].instance_VBO);
	glDeleteBuffers(1, &slices[1].instance_VBO);
    
    delete terrainShader;
    //delete camera;
    delete[] slices[0].mesh->memory_block;
	delete[] slices[1].mesh->memory_block;
    
    slices[0].mesh->verts = nullptr;
	slices[1].mesh->verts = nullptr;
    
    //slices[0].mesh->indices = nullptr;
    //slices[1].mesh->indices = nullptr;
    
    delete slices[0].mesh;
    delete slices[1].mesh;
}
