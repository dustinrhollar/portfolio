//
// Created by Dustin Hollar on 10/30/18.
//

#define EROSION_ITERATIONS 20

// a bilinear interpolation function used for finding texture coordinates
static glm::vec2 bilerp( int x, int y, int width, int height ) {
    
    glm::vec2 TL( 0, 1 );
    glm::vec2 TR( 1, 1 );
    glm::vec2 BL( 0, 0 );
    glm::vec2 BR( 1, 0 );
    
    float t = (float)y / (float)height;
    float s = (float)x / (float)width;
    
    glm::vec2 l = TL + s * ( BL - TL );
    glm::vec2 r = TR + s * ( BR - TR );
    
    return l + t * ( r - l );
}

Terrain::Terrain( unsigned int width, unsigned int height, unsigned int scale,
                 NoiseOctaveSimulation simulation ) :
terrainWidth( width ), terrainHeight( height ), terrainHeightScale( scale ) {
    createHeightmap( simulation );
    createBaseMesh();
    setUpGraphics();
}

Terrain::~Terrain() {
    
    delete[] heightmap;
    delete[] normalmap;
    delete[] terrainMesh;
    
    // TODO(Dustin): Fix.
    //glDeleteVertexArrays( 1, &VAO     );
    //glDeleteBuffers     ( 1, &VBO     );
    //glDeleteTextures    ( 1, &heightmapTexture );
    //glDeleteTextures    ( 1, &normalTexture );
}

// Create the heightmap using noise and erosion simulations
void Terrain::createHeightmap( NoiseOctaveSimulation noiseSim ) {
    
    // --------------------------------------------------------------------------------------------- //
    // Simulate noise
    // --------------------------------------------------------------------------------------------- //
    textureWidth  = (unsigned int)noiseSim.width;
    textureHeight = (unsigned int)noiseSim.height;
    std::cout << "Simulating Noise..." << std::endl;
    heightmap = simulateNoise( noiseSim );
    
    // --------------------------------------------------------------------------------------------- //
    // Simulate erosion
    // --------------------------------------------------------------------------------------------- //
#if THERMAL
    std::cout << "Simulating thermal erosion..." << std::endl;
    
    // Create Thermal Erosion Simulation Struct
    ThermalErosionSimlation simulation;
    simulation.width            = noiseSim.width;
    simulation.height           = noiseSim.height;
    simulation.numberIterations = EROSION_ITERATIONS;
    simulation.noiseMap         = heightmap;
    
    // Run the simulation
    simulateThermalErosion( simulation );
    //#endif
    
#elif HYDRAULIC
    std::cout << "Simulating hydraulic erosion..." << std::endl;
    
    // Set up constants for Erosion coefficients and struct
    float kr = 0.01f;           // Rain constant
    float ks = 0.01f;           // solubility constant of the terrain
    float ke = 0.5f;            // evaporation coefficient
    float kc = 0.01f;           // sediment transfer maximum coefficient
    
    ErosionCoefficient coeffErosion;
    coeffErosion.Kr = kr;
    coeffErosion.Ks = ks;
    coeffErosion.Ke = ke;
    coeffErosion.Kc = kc;
    
    // Create Hydraulic Erosion Simulation Struct
    HydraulicErosionSimulation simulation;
    simulation.erosionCoeffStruct = &coeffErosion;
    simulation.width              = noiseSim.width;
    simulation.height             = noiseSim.height;
    simulation.noiseMap           = heightmap;
    simulation.numberIterations   = EROSION_ITERATIONS;
    
    // Run the simulation
    simulateHydraulicErosion( simulation );
    //#endif
    
#elif INVERSETHERMAL
    std::cout << "Simulating inverse thermal erosion..." << std::endl;
    
    // Create Thermal Erosion Simulation Struct
    ThermalErosionSimlation simulation;
    simulation.width            = noiseSim.width;
    simulation.height           = noiseSim.height;
    simulation.numberIterations = EROSION_ITERATIONS;
    simulation.noiseMap         = heightmap;
    
    // Run the simulation
    simulateInverseThermalErosion( simulation );
#endif
    
    // --------------------------------------------------------------------------------------------- //
    // Write to the ppm file
    // --------------------------------------------------------------------------------------------- //
#if WRITE
    
    std::cout << "Writing to image file..." << std::endl;
    std::ofstream ofs;
    
    // Filename buffer
    char buff[22];
    
    // Define the name of the output file
#if NOISE
    std::cout << "Writing to noise image file..." << std::endl;
#if COLOR
    sprintf( buff, "images/noiseC%d.ppm", noiseSim.numOctaves );
    ofs.open( buff, std::ios::out | std::ios::binary );
#else
    sprintf( buff, "images/noiseG%d.ppm", noiseSim.numOctaves );
    ofs.open( buff, std::ios::out | std::ios::binary );
#endif
    //#endif
    
#elif THERMAL
    std::cout << "Writing to thermal image file..." << std::endl;
#if COLOR
    sprintf( buff, "images/thermC%d.ppm", numberIterations );
    ofs.open( buff, std::ios::out | std::ios::binary );
#else
    sprintf( buff, "images/thermG%d.ppm", numberIterations );
    ofs.open( buff, std::ios::out | std::ios::binary );
#endif
    //#endif
    
#elif HYDRAULIC
#ifdef COLOR
    sprintf( buff, "images/hydroC%d.ppm", numberIterations );
    ofs.open( buff, std::ios::out | std::ios::binary );
#else
    sprintf( buff, "images/hydroG%d.ppm", numberIterations );
    ofs.open( buff, std::ios::out | std::ios::binary );
#endif
    //#endif
    
#elif INVERSETHERMAL
#ifdef COLOR
    sprintf( buff, "images/invThC%3d.ppm", numberIterations );
    ofs.open( buff, std::ios::out | std::ios::binary );
#else
    sprintf( buff, "images/invThG%3d.ppm", numberIterations );
    ofs.open( buff, std::ios::out | std::ios::binary );
#endif
#endif
    
    ofs << "P6\n" << noiseSim.width << " " << noiseSim.height << "\n255\n";
    int length = noiseSim.width * noiseSim.height;
    for (int k = 0; k < length; ++k) {
        
#if COLOR
        BiomeColor bc;
        int biomereturn = biome( heightmap[ k ] );
        
        if ( biomereturn == WATER ) {
            ofs << bc.water[0] << bc.water[1] << bc.water[2];
        }
        else if ( biomereturn == BEACH ) {
            ofs << bc.beach[0] << bc.beach[1] << bc.beach[2];
        }
        else if ( biomereturn == FOREST ) {
            ofs << bc.forest[0] << bc.forest[1] << bc.forest[2];
        }
        else if ( biomereturn == JUNGLE ) {
            ofs << bc.jungle[0] << bc.jungle[1] << bc.jungle[2];
        }
        else if ( biomereturn == SAVANNAH ) {
            ofs << bc.savannah[0] << bc.savannah[1] << bc.savannah[2];
        }
        else if ( biomereturn == DESERT ) {
            ofs << bc.desert[0] << bc.desert[1] << bc.desert[2];
        }
        else if ( biomereturn == SNOW ) {
            ofs << bc.snow[0] << bc.snow[1] << bc.snow[2];
        }
        else { /* this better not ever happen */ }
        
#else
        // Print greyscale
        unsigned char n = heightmap[ k ] * 255;
        ofs << n << n << n;
#endif
    }
    ofs.close();
#endif
    
    // --------------------------------------------------------------------------------------------- //
    // Generate Normals
    // --------------------------------------------------------------------------------------------- //
    std::cout << "Generating normals..." << std::endl;
    normalmap = generateHeighmapNormals( heightmap, (unsigned int)noiseSim.width, (unsigned int)noiseSim.height );
    
    // --------------------------------------------------------------------------------------------- //
    // Write Normals to ppm file
    // --------------------------------------------------------------------------------------------- //
#ifdef WRITE
    std::cout << "Writing normals to ppm file..." << std::endl;
    std::ofstream ofn;
    ofn.open( "./images/normals.ppm", std::ios::out | std::ios::binary );
    ofn << "P6\n" << noiseSim.width << " " << noiseSim.height << "\n255\n";
    for ( int k = 0; k < length; k += 3) {
        unsigned char n  = ((normalmap[ k   ]*2)-1) * 255;
        unsigned char n2 = ((normalmap[ k + 1  ]*2)-1) * 255;
        unsigned char n3 = ((normalmap[ k + 2  ]*2)-1) * 255;
        ofn << n <<
            n2 <<
            n3;
    }
    ofn.close();
    
    ofn.open( "./images/normalstxt.txt", std::ios::out | std::ios::binary );
    ofn << "P6\n" << noiseSim.width << " " << noiseSim.height << "\n255\n";
    for (int k = 0; k < length; k += 3) {
        ofn << normalmap[ k     ] << " " <<
            normalmap[ k + 1 ] << " " <<
            normalmap[ k + 2 ] << "\n";
    }
    ofn.close();
#endif
    
    std::cout << "Completed terrain simulation." << std::endl;
}

// Create the terrain mesh
void Terrain::createBaseMesh() {
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // 6 : verts per quad, 3 : 3 data points per vert, 2 : 2 texture coords
    size = terrainWidth * terrainHeight * 6 * 6;
    terrainMesh = new float[ size ];
    
    int idx = 0;
    for (unsigned int i = 0; i < terrainWidth; ++i ) {
        for(unsigned int j = 0; j < terrainHeight; ++j ) {
            
            // Perform bi-linear interpolation to the the texture coordinate of each corner of the quad
            glm::vec2 TR = bilerp( i    , j + 1, terrainWidth, terrainHeight );
            glm::vec2 BR = bilerp( i + 1, j + 1, terrainWidth, terrainHeight );
            glm::vec2 TL = bilerp( i    , j    , terrainWidth, terrainHeight );
            glm::vec2 BL = bilerp( i + 1, j    , terrainWidth, terrainHeight );
            
            // TRIANGLE 1
            // Vertex 1, TR: i,j + 1
            terrainMesh[ idx++ ] = (i);// / height;
            terrainMesh[ idx++ ] = (j + 1);// / width;
            //terrainMesh[ idx++ ] = (0);
            terrainMesh[ idx++ ] = (TR[0]);
            terrainMesh[ idx++ ] = (TR[1]);
            terrainMesh[ idx++ ] = 1.0f;
            terrainMesh[ idx++ ] = 1.0f;
            
            // Vertex 2, BR: i + 1, j + 1
            terrainMesh[ idx++ ] = (i + 1);// / height;
            terrainMesh[ idx++ ] = (j + 1);// / width;
            //terrainMesh[ idx++ ] = (0);
            terrainMesh[ idx++ ] = BR[0];
            terrainMesh[ idx++ ] = BR[1];
            terrainMesh[ idx++ ] = 1.0f;
            terrainMesh[ idx++ ] = 0.0f;
            
            // Vertex 3, TL: i, j
            terrainMesh[ idx++ ] = (i);// / height;
            terrainMesh[ idx++ ] = (j);// / width;
            //terrainMesh[ idx++ ] =  0;
            terrainMesh[ idx++ ] = TL[0];
            terrainMesh[ idx++ ] = TL[1];
            terrainMesh[ idx++ ] = 0.0f;
            terrainMesh[ idx++ ] = 1.0f;
            
            // Triangle 2
            // Vertex 1, BR: i + 1, j + 1
            terrainMesh[ idx++ ] = (i + 1);// / height;
            terrainMesh[ idx++ ] = (j + 1);// / width;
            //terrainMesh[ idx++ ] = 0;
            terrainMesh[ idx++ ] = BR[0];
            terrainMesh[ idx++ ] = BR[1];
            terrainMesh[ idx++ ] = 1.0f;
            terrainMesh[ idx++ ] = 0.0f;
            
            // Vertex 2, BL: i + 1, j
            terrainMesh[ idx++ ] = (i + 1);// / height;
            terrainMesh[ idx++ ] = (j);// / width;
            //terrainMesh[ idx++ ] = 0;
            terrainMesh[ idx++ ] = BL[0];
            terrainMesh[ idx++ ] =  BL[1];
            terrainMesh[ idx++ ] = 0.0f;
            terrainMesh[ idx++ ] = 0.0f;
            
            // Vertex 3, TL: i, j
            terrainMesh[ idx++ ] = (i);// / height;
            terrainMesh[ idx++ ] = (j);// / width;
            //terrainMesh[ idx++ ] = 0;
            terrainMesh[ idx++ ] = TL[0];
            terrainMesh[ idx++ ] = TL[1];
            terrainMesh[ idx++ ] = 0.0f;
            terrainMesh[ idx++ ] = 1.0f;
        }
    }
}

void Terrain::setUpGraphics() {
    pipelineId = createPipeline(false,"shaders/geometryVertexShader.vs", 
                                "shaders/geometryFragmentShader.fs");
    Pipeline *pipeline = GetPipeline(pipelineId);
    
    CreateMesh(&mesh, pipelineId, false);
    //int components = 4 * (2 + 4);
    mesh.size = ( size * sizeof( float ) ) / 24;
    
    glBindVertexArray(mesh.VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, size * sizeof( float ), terrainMesh, GL_STATIC_DRAW);
    
    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture attribute
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)( 2 * sizeof(float) ) );
    glEnableVertexAttribArray(1);
    
    // load and create a heightmap texture
    // -------------------------
    createTexture(&pipeline->diffuseTexture, textureWidth, textureHeight,
                  GL_R16F, GL_RED, GL_FLOAT, (char*)heightmap);
    
    // load and create a normalmap texture
    // -------------------------
    createTexture(&pipeline->normalTexture, textureWidth, textureHeight,
                  GL_RGB, GL_RGB, GL_FLOAT, (char*)normalmap);
    
    // load and create a material texture
    // -------------------------
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = stbi_load("images/dirt.png", &width, &height, &nrChannels, 0);
    createTexture(&pipeline->albedoTexture, width, height,
                  GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, (char*)data);
    stbi_image_free(data);
}

// Render the terrain
void Terrain::render(glm::mat4 projection, glm::mat4 view) {
    Pipeline *pipeline = GetPipeline(pipelineId);;
    Shader *geometryShader = pipeline->geometryShader;
    
    geometryShader->use();
    geometryShader->setMat4("projection", projection);
    geometryShader->setMat4("view", view);
    
    // bind the VAO
    glBindVertexArray(mesh.VAO);
    
    // bind Texture
    glActiveTexture( GL_TEXTURE0 );
    glUniform1i(glGetUniformLocation(geometryShader->ID, "heightTexture"), 0);
    glBindTexture( GL_TEXTURE_2D, pipeline->diffuseTexture.id );
    glActiveTexture( GL_TEXTURE1 );
    glUniform1i(glGetUniformLocation(geometryShader->ID, "normalTexture"), 1);
    glBindTexture( GL_TEXTURE_2D, pipeline->normalTexture.id );
    glActiveTexture( GL_TEXTURE2);
    glUniform1i(glGetUniformLocation(geometryShader->ID, "landscapeTexture"), 2);
    glBindTexture( GL_TEXTURE_2D, pipeline->albedoTexture.id );
    
    // Set the height scale
    geometryShader->setFloat("scale", terrainHeightScale );
    
    // calculate the model matrix for each object and pass it to shader before drawing
    glm::mat4 model = glm::mat4(1.0);
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    geometryShader->setMat4("model", model);
    
    int components = 4 * (2 + 4);
    glDrawArrays(GL_TRIANGLES, 0, mesh.size );
    
    glBindVertexArray(0);
    
    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}