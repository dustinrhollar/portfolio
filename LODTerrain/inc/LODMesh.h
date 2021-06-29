// Creates a terrain mesh for Level of Detail
// @author Dustin Hollar

#ifndef LODTEST_LODMESH_H
#define LODTEST_LODMESH_H

#include <vector>
#include <glm/glm.hpp>

typedef unsigned int uint;

enum Angle {
    DEGREES,
    RADIANS
};

struct LODMesh {
	// use a contiguous memory block - unnecessary for its current use
    char* memory_block = nullptr;
    float* verts = nullptr;
    
    // size of internal memory blocks
    uint num_vert = 0;
    uint size_vert = 0;
};

// Generates a single quadrant of the mesh
LODMesh* generateFullMesh( uint width, uint height, uint patch_size );

// Generates a portion of the mesh using a specified arc.
LODMesh* generateSlice( uint cols, uint patch, Angle type, float lowerAngle, float upperAngle );

#endif //LODTEST_LODMESH_H
