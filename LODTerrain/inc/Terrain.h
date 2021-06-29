//
// Created by Dustin Hollar on 10/30/18.
//

#ifndef TERRAINGENERATORTEST_TERRAIN_H
#define TERRAINGENERATORTEST_TERRAIN_H

using namespace Erosion;
using namespace SimplexNoise;
using namespace Biome;

class Terrain {
    
    private:
    unsigned int terrainWidth;
    unsigned int terrainHeight;
    unsigned int terrainHeightScale;
    
    unsigned int textureWidth;
    unsigned int textureHeight;
    
    float* heightmap;
    float* normalmap;
    float* terrainMesh;
    
    int pipelineId;
    Mesh mesh;
    int size;
    
    void createHeightmap( NoiseOctaveSimulation simulation );
    void createBaseMesh();
    void setUpGraphics();
    
    public:
    Terrain( unsigned int, unsigned int, unsigned int, NoiseOctaveSimulation);
    ~Terrain();
    void render(glm::mat4 projection, glm::mat4 view);
};


#endif //TERRAINGENERATORTEST_TERRAIN_H
